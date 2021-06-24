#include "stagethree.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocostudio::timeline;

StageThree::~StageThree()
{
#ifdef BOX2D_DEBUG
	if (_DebugDraw != NULL) delete _DebugDraw;
#endif

	if (_b2World != nullptr) delete _b2World;
	//  for releasing Plist&Texture
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
	SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("scene101bg.plist");
	Director::getInstance()->getTextureCache()->removeUnusedTextures();
}

Scene* StageThree::createScene()
{
	return StageThree::create();
}

bool StageThree::init()
{
	if (!Scene::init())
	{
		return false;
	}
	//  For Loading Plist+Texture
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("box2d.plist");
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("scene101bg.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// �إ� Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//���O��V
	bool AllowSleep = true;			//���\�ε�
	_b2World = new b2World(Gravity);	//�Ыإ@��
	_b2World->SetAllowSleeping(AllowSleep);	//�]�w���󤹳\�ε�

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("stagethree.csb");
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);
#endif
	addChild(_csbRoot, 1);

	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("returnbtn"));
	_returnButton = CButton::create();
	_returnButton->setButtonInfo("returnbtn.png", "returnbtn.png", btnSprite->getPosition());
	_returnButton->setScale(btnSprite->getScale());
	this->addChild(_returnButton, 3);
	btnSprite->setVisible(false);
	_bToStartScene = false;

	boxLayer = Layer::create();
	this->addChild(boxLayer);

	target = RenderTexture::create(_visibleSize.width, _visibleSize.height, kCCTexture2DPixelFormat_RGBA8888);
	target->retain();
	target->setPosition(Vec2(_visibleSize.width / 2 + origin.x, _visibleSize.height / 2 + origin.y));

	boxLayer->addChild(target);

	brush = Sprite::create("brush.png");
	//brush->setVisible(false);
	brush->retain();

	createStaticBoundary();
	test = 0;

#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//�]�wDebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//���ø�s���O
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//ø�s�Ϊ�
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//�]�wø�s����
	_DebugDraw->SetFlags(flags);
#endif

	//_b2World->SetContactListener(&_contactListener);

	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(StageThree::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(StageThree::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(StageThree::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(StageThree::doStep));

	return true;
}

void StageThree::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	if (_bToStartScene)
	{
		log("return");
		// ���N�o�� SCENE �� update�q schedule update �����X
		this->unschedule(schedule_selector(StageThree::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("gamescene.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		TransitionFade* pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
	_b2World->Step(dt, velocityIterations, positionIterations);
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		// body->ApplyForce(b2Vec2(10.0f, 10.0f), body->GetWorldCenter(), true);
		// �H�U�O�H Body ���]�t Sprite ��ܬ���
		if (body->GetUserData() != NULL)
		{
			Sprite* ballData = static_cast<Sprite*>(body->GetUserData());
			ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
			ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
		}
	}
}

bool StageThree::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	log("stage Three return btn");
	_returnButton->touchesBegin(touchLoc);

	log("touch began");
	int r = rand() % 128 + 128;
	int b = rand() % 128 + 128;
	int g = rand() % 128 + 128;
	brush->setColor(ccc3(r, b, g));

	plataformPoints.clear();
	if (plataformPoints.empty())log("empty");
	Point location = pTouch->getLocation();

	plataformPoints.push_back(location);
	previousLocation = location;

	b2BodyDef myBodyDef;
	myBodyDef.type = b2_staticBody;
	myBodyDef.position.Set(location.x / PTM_RATIO, location.y / PTM_RATIO);
	currentPlatformBody = _b2World->CreateBody(&myBodyDef);

	return true;
}

void StageThree::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	_returnButton->touchesMoved(touchLoc);

	Point start = pTouch->getLocation();
	Point end = pTouch->getPreviousLocation();

	target->begin();

	float distance = ccpDistance(start, end);

	for (int i = 0; i < distance; i++)
	{
		float difx = end.x - start.x;
		float dify = end.y - start.y;
		float delta = (float)i / distance;
		brush->setPosition(ccp(start.x + (difx * delta), start.y + (dify * delta)));
		brush->visit();
	}
	target->end();

	float distance2 = ccpDistance(start, previousLocation);
	if (distance2 > 15.0f)
	{
		plataformPoints.push_back(start);
		previousLocation = start;
	}
}

void StageThree::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	if (_returnButton->touchesEnded(touchLoc))
	{
		_bToStartScene = true;
	}

	Size s = Director::sharedDirector()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	if (plataformPoints.size() > 1) {

		//Add a new body/atlas sprite at the touched location
		b2BodyDef myBodyDef;
		myBodyDef.type = b2_dynamicBody; //this will be a dynamic body
		myBodyDef.position.Set(currentPlatformBody->GetPosition().x, currentPlatformBody->GetPosition().y); //set the starting position
		myBodyDef.userData = NULL;
		b2Body* newBody = _b2World->CreateBody(&myBodyDef);
		std::ostringstream ostr;
		std::string objname;
		ostr << "brush_0" << test; objname = ostr.str();

		for (int i = 0; i < plataformPoints.size() - 1; i++)
		{
			Point start = plataformPoints[i];
			Point end = plataformPoints[i + 1];
			this->addRectangle(newBody, start, end);
		}

		_b2World->DestroyBody(currentPlatformBody);


		Rect bodyRectangle = this->getBodyRectangle(newBody);
		Texture2D* tex = nullptr;

		Image* pImage = nullptr;
		if (pImage == nullptr)
		{
			pImage = target->newImage();
			TextureCache::getInstance()->addImage(pImage, objname);
			tex = TextureCache::getInstance()->getTextureForKey(objname);
			CC_SAFE_DELETE(pImage);
		}
		else
		{
			TextureCache::getInstance()->addImage(pImage, objname);
			tex = TextureCache::getInstance()->getTextureForKey(objname);
			CC_SAFE_DELETE(pImage);
		}


		Sprite* sprite = nullptr;
		if (sprite == nullptr)
		{
			sprite = new Sprite();
			sprite->initWithTexture(tex, bodyRectangle);
		}
		else
		{
			sprite->initWithTexture(tex, bodyRectangle);
		}

		if (newBody->GetUserData() == NULL)
		{
			float anchorX = newBody->GetPosition().x * PTM_RATIO - bodyRectangle.origin.x;
			float anchorY = bodyRectangle.size.height - (s.height - bodyRectangle.origin.y - newBody->GetPosition().y * PTM_RATIO);

			sprite->setAnchorPoint(Vec2(anchorX / bodyRectangle.size.width, anchorY / bodyRectangle.size.height));
			newBody->SetUserData(sprite);
			log("%1.3f", anchorX);
			log("%1.3f", anchorY);
			boxLayer->addChild(sprite);
		}
		test++;
	}
	boxLayer->removeChild(target, true);
	target->release();

	target = RenderTexture::create(_visibleSize.width, _visibleSize.height, kCCTexture2DPixelFormat_RGBA8888);
	target->retain();
	target->setPosition(Vec2(_visibleSize.width / 2 + origin.x, _visibleSize.height / 2 + origin.y));
	boxLayer->addChild(target);
}

#ifdef BOX2D_DEBUG
//��gø�s��k
void StageThree::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

void StageThree::createStaticBoundary()
{
	// ������ Body, �]�w�������Ѽ�

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // �]�w�o�� Body �� �R�A��
	bodyDef.userData = NULL;
	// �b b2World �����͸� Body, �öǦ^���ͪ� b2Body ���󪺫���
	// ���ͤ@���A�N�i�H���᭱�Ҧ��� Shape �ϥ�
	b2Body* body = _b2World->CreateBody(&bodyDef);

	// �����R�A��ɩһݭn�� EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // ���� Fixture
	edgeFixtureDef.shape = &edgeShape;
	// bottom edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// left edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// right edge
	edgeShape.Set(b2Vec2(_visibleSize.width / PTM_RATIO, 0.0f / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);

	// top edge
	edgeShape.Set(b2Vec2(0.0f / PTM_RATIO, _visibleSize.height / PTM_RATIO), b2Vec2(_visibleSize.width / PTM_RATIO, _visibleSize.height / PTM_RATIO));
	body->CreateFixture(&edgeFixtureDef);
}

void StageThree::addRectangle(b2Body* body, Point start, Point end)
{
	float min = brush->getContentSize().width * brush->getScale() / PTM_RATIO;

	float dist_x = start.x - end.x;
	float dist_y = start.y - end.y;
	float angle = atan2(dist_y, dist_x);
	float px = (start.x + end.x) / 2 / PTM_RATIO - body->GetPosition().x;
	float py = (start.y + end.y) / 2 / PTM_RATIO - body->GetPosition().y;
	float width = MAX(abs(dist_x) / PTM_RATIO, min);
	float height = MAX(abs(dist_y) / PTM_RATIO, min);

	b2PolygonShape boxShape;
	boxShape.SetAsBox(width / 2, height / 2, b2Vec2(px, py), angle);

	b2FixtureDef boxFixtureDef;
	boxFixtureDef.shape = &boxShape;
	boxFixtureDef.density = 5;
	//boxFixtureDef.restitution = 1;

	body->CreateFixture(&boxFixtureDef);
}

Rect StageThree::getBodyRectangle(b2Body* body)
{
	Size s = Director::getInstance()->getWinSize();

	float minX = s.width;
	float maxX = 0;
	float minY = s.height;
	float maxY = 0;

	const b2Transform& xf = body->GetTransform();
	for (b2Fixture* f = body->GetFixtureList(); f; f = f->GetNext())
	{
		b2PolygonShape* poly = (b2PolygonShape*)f->GetShape();
		int32 vertexCount = poly->m_count;// maybe have problem
		b2Assert(vertexCount <= b2_maxPolygonVertices);
		//log("%d", vertexCount);
		for (int32 i = 0; i < vertexCount; ++i)
		{
			b2Vec2 vertex = b2Mul(xf, poly->m_vertices[i]);

			minX = MIN(minX, vertex.x);
			maxX = MAX(maxX, vertex.x);
			minY = MIN(minY, vertex.y);
			maxY = MAX(maxY, vertex.y);
		}
	}

	maxX *= PTM_RATIO;
	minX *= PTM_RATIO;
	maxY *= PTM_RATIO;
	minY *= PTM_RATIO;


	float margin = brush->getContentSize().width * brush->getScale() / 2.0f;

	float width = maxX - minX + margin * 2;
	float height = maxY - minY + margin * 2;

	float x = minX - margin;
	float y = s.height - maxY - margin;


	x = MAX(0.0f, x);
	y = MAX(0.0f, y);
	if (minX + width > s.width) {
		width = s.width - x;
	}
	if (minY + height > s.height) {
		height = s.height - y;
	}

	return CCRectMake(x, y, width, height);
}