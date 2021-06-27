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
	Director::getInstance()->getTextureCache()->removeAllTextures();
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
	SpriteFrameCache::getInstance()->addSpriteFramesWithFile("particletexture.plist");

	_visibleSize = Director::getInstance()->getVisibleSize();
	Vec2 origin = Director::getInstance()->getVisibleOrigin();

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("stagethree.csb");
	_endNode = CSLoader::createNode("endNode.csb");
#ifndef BOX2D_DEBUG
	// 設定顯示背景圖示
	auto bgSprite = _csbRoot->getChildByName("bg64_1");
	bgSprite->setVisible(true);
#endif
	addChild(_csbRoot, 1);
	addChild(_endNode, 10);

	auto btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("returnbtn"));
	_returnButton[0] = CButton::create();
	_returnButton[0]->setButtonInfo("returnbtn.png", "returnbtn.png", btnSprite->getPosition());
	_returnButton[0]->setScale(btnSprite->getScale());
	this->addChild(_returnButton[0], 3);
	btnSprite->setVisible(false);
	_bToStartScene = false;

	btnSprite = dynamic_cast<Sprite*>(_endNode->getChildByName("returnbtn"));
	_returnButton[1] = CButton::create();
	_returnButton[1]->setButtonInfo("returnbtn.png", "returnbtn.png", btnSprite->getPosition());
	_returnButton[1]->setScale(btnSprite->getScale());
	_returnButton[1]->setVisible(false);
	this->addChild(_returnButton[1], 3);
	btnSprite->setVisible(false);

	btnSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("resetbtn"));
	_resetButton[0] = CButton::create();
	_resetButton[0]->setButtonInfo("replaybtn.png", "replaybtn.png", btnSprite->getPosition());
	_resetButton[0]->setScale(btnSprite->getScale());
	this->addChild(_resetButton[0], 3);
	btnSprite->setVisible(false);

	btnSprite = dynamic_cast<Sprite*>(_endNode->getChildByName("resetbtn"));
	_resetButton[1] = CButton::create();
	_resetButton[1]->setButtonInfo("replaybtn.png", "replaybtn.png", btnSprite->getPosition());
	_resetButton[1]->setScale(btnSprite->getScale());
	_resetButton[1]->setVisible(false);
	this->addChild(_resetButton[1], 3);
	btnSprite->setVisible(false);

	boxLayer = Layer::create();
	this->addChild(boxLayer , 5);

	target = RenderTexture::create(_visibleSize.width, _visibleSize.height, kCCTexture2DPixelFormat_RGBA8888);
	target->retain();
	target->setPosition(Vec2(_visibleSize.width / 2 + origin.x, _visibleSize.height / 2 + origin.y));

	boxLayer->addChild(target);

	brush = Sprite::create("brush.png");
	//brush->setVisible(false);
	brush->retain();

	createStaticBoundary();
	createRock();
	createCar();
	createSensor(1, 0);
	createSensor(3, 1);
	createWall();
	createBasketandPulley();
	test = 0;
	_breset = false;

#ifdef BOX2D_DEBUG
	//DebugDrawInit
	_DebugDraw = nullptr;
	_DebugDraw = new GLESDebugDraw(PTM_RATIO);
	//設定DebugDraw
	_b2World->SetDebugDraw(_DebugDraw);
	//選擇繪製型別
	uint32 flags = 0;
	flags += GLESDebugDraw::e_shapeBit;						//繪製形狀
	flags += GLESDebugDraw::e_pairBit;
	flags += GLESDebugDraw::e_jointBit;
	flags += GLESDebugDraw::e_centerOfMassBit;
	flags += GLESDebugDraw::e_aabbBit;
	//設定繪製類型
	_DebugDraw->SetFlags(flags);
#endif

	_b2World->SetContactListener(&_contactListener);

	_shapeCreator = new StaticShapeCreator;
	_shapeCreator->init(*_b2World, _visibleSize, *this, *_csbRoot);
	_shapeCreator->createShape(2, 4);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(StageThree::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(StageThree::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(StageThree::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(StageThree::doStep));

	return true;
}

void StageThree::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	if (_bToStartScene)
	{
		log("return");
		// 先將這個 SCENE 的 update從 schedule update 中移出
		this->unschedule(schedule_selector(StageThree::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("gamescene.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		TransitionFade* pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (!_contactListener.isGoal())
	{
		_b2World->Step(dt, velocityIterations, positionIterations);
		for (b2Body* body = _b2World->GetBodyList(); body; /*body = body->GetNext()*/)
		{
			// body->ApplyForce(b2Vec2(10.0f, 10.0f), body->GetWorldCenter(), true);
			// 以下是以 Body 有包含 Sprite 顯示為例
			if (body->GetUserData() != NULL)
			{
				Sprite* ballData = static_cast<Sprite*>(body->GetUserData());
				ballData->setPosition(body->GetPosition().x * PTM_RATIO, body->GetPosition().y * PTM_RATIO);
				ballData->setRotation(-1 * CC_RADIANS_TO_DEGREES(body->GetAngle()));
			}
			if (body->GetType() == b2BodyType::b2_dynamicBody) {
				float x = body->GetPosition().x * PTM_RATIO;
				float y = body->GetPosition().y * PTM_RATIO;
				if (x > _visibleSize.width || x < 0 || y >  _visibleSize.height || y < 0) {
					if (body->GetUserData() != NULL) {
						Sprite* spriteData = (Sprite*)(body->GetUserData());
						this->removeChild(spriteData);
					}
					b2Body* nextbody = body->GetNext(); // 取得下一個 body
					_b2World->DestroyBody(body); // 釋放目前的 body
					body = nextbody;  // 讓 body 指向剛才取得的下一個 body
				}
				else body = body->GetNext(); //否則就繼續更新下一個Body
			}
			else body = body->GetNext(); //否則就繼續更新下一個Body
		}
	}

	if (_contactListener.isGoal())
	{
		_returnButton[1]->setVisible(true);
		_resetButton[1]->setVisible(true);
	}
	if (_contactListener.isPortal())
	{
		float randomLocX =70 - (rand() % 140);
		float randomLocY = 20 - (rand() % 40);
		Sprite* RockData = static_cast<Sprite*>(_contactListener.getContactObj()->GetUserData());
		Sprite* destination = static_cast<Sprite*>(_csbRoot->getChildByName("portal01_02"));
		RockData->setPosition((destination->getPosition().x + randomLocX) * PTM_RATIO, (destination->getPosition().y + randomLocY) * PTM_RATIO);
		_contactListener.getContactObj()->SetTransform(b2Vec2((destination->getPosition().x + randomLocX) / PTM_RATIO, (destination->getPosition().y + randomLocY) / PTM_RATIO), 0);
		if (_contactListener.getContactObj()->GetLinearVelocity().Length() >= 20.0f)
		{
			_contactListener.getContactObj()->SetLinearVelocity(b2Vec2(10 - (rand() % 21), -(rand() % 21)));
		}
	}

	if (_rearWheel->GetAngularVelocity() < -4.0f)
	{
		auto sparkSprite = Sprite::createWithSpriteFrameName("cloud.png");
		//sparkSprite->setColor(Color3B(rand() % 256, rand() % 256, rand() % 156));
		sparkSprite->setBlendFunc(BlendFunc::ADDITIVE);
		this->addChild(sparkSprite, 10);
		//產生小方塊資料
		b2BodyDef RectBodyDef;
		RectBodyDef.position.Set(_rearWheel->GetPosition().x + ((30 - rand() % 60) / PTM_RATIO), _rearWheel->GetPosition().y);
		RectBodyDef.type = b2_dynamicBody;
		RectBodyDef.userData = sparkSprite;
		RectBodyDef.gravityScale = -0.25f;
		b2CircleShape CircleShape;
		CircleShape.m_radius = sparkSprite->getContentSize().width * 0.25 / PTM_RATIO;
		b2PolygonShape RectShape;
		RectShape.SetAsBox(5 / PTM_RATIO, 5 / PTM_RATIO);
		b2Body* RectBody = _b2World->CreateBody(&RectBodyDef);
		b2FixtureDef RectFixtureDef;
		RectFixtureDef.shape = &CircleShape;
		//RectFixtureDef.density = 1.0f;
		RectFixtureDef.isSensor = true;
		b2Fixture* RectFixture = RectBody->CreateFixture(&RectFixtureDef);

		//給力量
		RectBody->ApplyForce(b2Vec2(rand() % 11 - 20, 5 - rand() % 10), _rearWheel->GetPosition(), true);
	}
}

bool StageThree::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	_breset = false;
	Point touchLoc = pTouch->getLocation();
	log("stage Three return btn");
	for (int i = 0; i < 2; i++)
	{
		_returnButton[i]->touchesBegin(touchLoc);
		_resetButton[i]->touchesBegin(touchLoc);
	}

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
	for (int i = 0; i < 2; i++)
	{
		_returnButton[i]->touchesMoved(touchLoc);
		_resetButton[i]->touchesMoved(touchLoc);
	}
	

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
	for (int i = 0; i < 2; i++)
	{
		if (_returnButton[i]->touchesEnded(touchLoc))
		{
			_bToStartScene = true;
		}
	}
	if (_resetButton[0]->touchesEnded(touchLoc))
	{
		reset();
	}
	else if (_resetButton[1]->touchesEnded(touchLoc))
	{
		reset();
		resetObj();
	}
	if (_breset)
	{
		for (vector<b2Body*>::iterator first = freehandBody.begin(); first != freehandBody.end(); ++first)
		{
			_b2World->DestroyBody(*first);
		}
		freehandBody.clear();
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

		freehandBody.push_back(newBody);

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
//改寫繪製方法
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
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

	// 產生靜態邊界所需要的 EdgeShape
	b2EdgeShape edgeShape;
	b2FixtureDef edgeFixtureDef; // 產生 Fixture
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
	boxFixtureDef.density = 4;
	boxFixtureDef.filter.maskBits = 0 << 1 | 1;
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

void StageThree::createRock()
{
	std::ostringstream ostr;
	std::string objname;

	for (int i = 1; i <= 5; i++)
	{
		ostr.str("");
		ostr << "rock_0" << i; objname = ostr.str();
		// 先建立 ballSprite 的 Sprite 並加入場景中
		auto ballSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		ballSprite->setScale(0.75f);
		Point loc = ballSprite->getPosition();
		_spawnPoint[i] = ballSprite->getPosition();
		float ballScale = ballSprite->getScale();

		// 建立一個簡單的動態球體
		b2BodyDef bodyDef;	// 先以結構 b2BodyDef 宣告一個 Body 的變數
		bodyDef.type = b2_dynamicBody; // 設定為動態物體
		bodyDef.userData = ballSprite;	// 設定 Sprite 為動態物體的顯示圖示
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		// 以 bodyDef 在 b2World  中建立實體並傳回該實體的指標
		b2Body* ballBody = _b2World->CreateBody(&bodyDef);
		// 設定該物體的外型
		b2CircleShape ballShape;	//  宣告物體的外型物件變數，此處是圓形物體
		Size ballsize = ballSprite->getContentSize();	// 根據 Sprite 圖形的大小來設定圓形的半徑
		ballShape.m_radius = (ballsize.width * ballScale) * 0.5f / PTM_RATIO;
		// 以 b2FixtureDef  結構宣告剛體結構變數，並設定剛體的相關物理係數
		b2FixtureDef fixtureDef;	 // 固定裝置
		fixtureDef.shape = &ballShape;			// 指定剛體的外型為圓形
		fixtureDef.restitution = 0.4f;			// 設定恢復係數
		fixtureDef.density = 18.0f;				// 設定密度
		fixtureDef.friction = 0.4f;			// 設定摩擦係數
		fixtureDef.filter.maskBits = 1 << 2 | 1;
		ballBody->CreateFixture(&fixtureDef);	// 在 Body 上產生這個剛體的設定
		_Rock[i - 1] = ballBody;
	}
}

void StageThree::createBasketandPulley()
{
	std::ostringstream ostr;
	std::string objname;
	//basket
	b2Body* basketWall[5];	

	for (int i = 1; i <= 5; i++)
	{
		ostr.str("");
		ostr << "basket_0" << i; objname = ostr.str();
		auto basketSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Size size = basketSprite->getContentSize();
		Point loc = basketSprite->getPosition();
		float angle = basketSprite->getRotation();
		float scale = basketSprite->getScaleX();
		float calRadian = M_PI / (180.0f / (180.0f - angle));
		b2Body* body;
		b2BodyDef bodyDef;
		b2FixtureDef fixtureDef;
		b2PolygonShape edgeShape;

		

		bodyDef.type = b2_dynamicBody;
		bodyDef.userData = basketSprite;
		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);


		fixtureDef.shape = &edgeShape;

		edgeShape.SetAsBox((size.width - 4) * scale * 0.5 / PTM_RATIO, size.height * 0.5 / PTM_RATIO);
		
		if (i > 3)fixtureDef.filter.categoryBits = 0 << 1;
		
		body = _b2World->CreateBody(&bodyDef);
		body->CreateFixture(&fixtureDef);
		body->SetTransform(b2Vec2(loc.x / PTM_RATIO, loc.y / PTM_RATIO), calRadian);

		basketWall[i - 1] = body;
	}

	b2WeldJointDef JointDef;
	JointDef.Initialize(basketWall[1], basketWall[0], basketWall[1]->GetPosition() + b2Vec2(-80 / PTM_RATIO, 0));
	JointDef.frequencyHz = 0;
	JointDef.dampingRatio = 0;
	_b2World->CreateJoint(&JointDef);

	JointDef.Initialize(basketWall[1], basketWall[2], basketWall[1]->GetPosition() + b2Vec2(80 / PTM_RATIO, 0));
	JointDef.frequencyHz = 0;
	JointDef.dampingRatio = 0;
	_b2World->CreateJoint(&JointDef);

	JointDef.Initialize(basketWall[0], basketWall[3], basketWall[0]->GetPosition() + b2Vec2(0, 80 / PTM_RATIO));
	JointDef.frequencyHz = 0;
	JointDef.dampingRatio = 0;
	_b2World->CreateJoint(&JointDef);

	JointDef.Initialize(basketWall[2], basketWall[4], basketWall[2]->GetPosition() + b2Vec2(0, 80 / PTM_RATIO));
	JointDef.frequencyHz = 0;
	JointDef.dampingRatio = 0;
	_b2World->CreateJoint(&JointDef);

	JointDef.Initialize(basketWall[3], basketWall[4], basketWall[3]->GetPosition() + b2Vec2(0, 0));
	JointDef.frequencyHz = 0;
	JointDef.dampingRatio = 0;
	_b2World->CreateJoint(&JointDef);
	//.........basket

	//door
	auto doorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("door_01"));
	Size doorSize = doorSprite->getContentSize();
	_doorSpawn = doorSprite->getPosition();
	Point doorLoc = doorSprite->getPosition();
	float scaleX = doorSprite->getScaleX();
	float scaleY = doorSprite->getScaleY();

	b2BodyDef doorBodyDef;
	doorBodyDef.type = b2_dynamicBody;
	doorBodyDef.userData = doorSprite;
	doorBodyDef.position.Set(doorLoc.x / PTM_RATIO, doorLoc.y / PTM_RATIO);

	b2Body* doorBody = _b2World->CreateBody(&doorBodyDef);

	b2PolygonShape doorShape;
	doorShape.SetAsBox(doorSize.width * scaleX * 0.5f / PTM_RATIO, (doorSize.height - 4) * scaleY * 0.5f / PTM_RATIO);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &doorShape;
	fixtureDef.restitution = 0.1f;
	fixtureDef.density = 15.0f;
	fixtureDef.friction = 0.1f;
	doorBody->CreateFixture(&fixtureDef);

	_door = doorBody;
	//.........door

	b2PulleyJointDef PJointDef;
	PJointDef.Initialize(basketWall[3], doorBody,
		b2Vec2(basketWall[3]->GetPosition().x + (40 / PTM_RATIO), basketWall[3]->GetPosition().y + (350 / PTM_RATIO))
		, b2Vec2(doorBody->GetPosition().x, doorBody->GetPosition().y + (275 / PTM_RATIO))
		, b2Vec2(basketWall[3]->GetPosition().x + (40 / PTM_RATIO), basketWall[3]->GetPosition().y + (40 / PTM_RATIO))
		, b2Vec2(doorBody->GetPosition().x, doorBody->GetPosition().y + (200 / PTM_RATIO))
		, 1);
	_b2World->CreateJoint(&PJointDef);
}

void StageThree::createCar()
{
	b2Body* wheel[2];

	std::ostringstream ostr;
	std::string objname;
	//Car body
	auto carSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("car"));
	Point loc = carSprite->getPosition();
	_spawnPoint[0] = carSprite->getPosition();
	Size size = carSprite->getContentSize();
	float scaleX = carSprite->getScaleX();
	float scaleY = carSprite->getScaleY();

	b2BodyDef bodydef;
	bodydef.type = b2_dynamicBody;
	bodydef.userData = carSprite;
	bodydef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);

	b2Body* carBody = _b2World->CreateBody(&bodydef);

	b2PolygonShape carShape;
	b2FixtureDef fixtureDef;

	carShape.SetAsBox((size.width - 8) * 0.5f * scaleX / PTM_RATIO, (size.height - 8) * 0.25f * scaleY / PTM_RATIO);
	fixtureDef.shape = &carShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0.1f;
	fixtureDef.filter.maskBits = 1 << 1 | 1;
	carBody->CreateFixture(&fixtureDef);

	_player = carBody;
	//

	for (int i = 1; i <= 2; i++)
	{
		ostr.str("");
		ostr << "wheel_0" << i; objname = ostr.str();

		auto wheelSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		Point wheelLoc = wheelSprite->getPosition();
		Size wheelSize = wheelSprite->getContentSize();
		b2CircleShape wheelShape;
		wheelShape.m_radius = wheelSize.width * 0.5f / PTM_RATIO;
		bodydef.position.Set(wheelLoc.x / PTM_RATIO, wheelLoc.y / PTM_RATIO);
		bodydef.userData = wheelSprite;

		b2Body* wheelBody = _b2World->CreateBody(&bodydef);
		fixtureDef.shape = &wheelShape;
		fixtureDef.density = 1.0f;
		fixtureDef.friction = 0.2f;
		fixtureDef.restitution = 0.4f;

		wheelBody->CreateFixture(&fixtureDef);
		wheel[i - 1] = wheelBody;

		if (i == 1)_rearWheel = wheelBody;
	}

	b2RevoluteJointDef wheelJoint;
	wheelJoint.bodyA = carBody;
	wheelJoint.localAnchorA.Set(-1.2f, -0.7f);
	wheelJoint.bodyB = wheel[0];
	wheelJoint.localAnchorB.Set(0, 0);

	_b2World->CreateJoint(&wheelJoint);

	wheelJoint.localAnchorA.Set(1.0f, -0.7f);
	wheelJoint.bodyB = wheel[1];
	wheelJoint.localAnchorB.Set(0, 0);

	_b2World->CreateJoint(&wheelJoint);
}

void StageThree::createSensor(int type, int amount)
{
	std::ostringstream ostr;
	std::string objname;
	switch (type)
	{
	case 1:
		for (int i = 1; i <= 1; i++)
		{
			ostr.str("");
			ostr << "sensor0" << i; objname = ostr.str();
			auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Point loc = sensorSprite->getPosition();
			Size  size = sensorSprite->getContentSize();
			float scale = sensorSprite->getScale();
			sensorSprite->setVisible(false);
			b2BodyDef sensorBodyDef;
			sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
			sensorBodyDef.type = b2_staticBody;

			b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
			b2PolygonShape sensorShape;
			sensorShape.SetAsBox(size.width * 0.5f * scale / PTM_RATIO, size.height * 0.5f * scale / PTM_RATIO);

			b2FixtureDef SensorFixtureDef;
			SensorFixtureDef.shape = &sensorShape;
			SensorFixtureDef.isSensor = true;	// 設定為 Sensor
			SensorFixtureDef.density = 9999 + i; // 故意設定成這個值，方便碰觸時候的判斷
			SensorBody->CreateFixture(&SensorFixtureDef);
		}
		break;
	case 2:
		//for (int i = 2; i <= amount + 1; i++)
		//{
		//	ostr.str("");
		//	ostr << "sensor0" << i; objname = ostr.str();
		//	auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		//	Point loc = sensorSprite->getPosition();
		//	Size  size = sensorSprite->getContentSize();
		//	float scale = sensorSprite->getScale();
		//	sensorSprite->setColor(filterColor[i - 2]);
		//	b2BodyDef sensorBodyDef;
		//	sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
		//	sensorBodyDef.type = b2_staticBody;

		//	b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
		//	b2PolygonShape sensorShape;
		//	sensorShape.SetAsBox(size.width * 0.5f * scale / PTM_RATIO, size.height * 0.5f * scale / PTM_RATIO);

		//	b2FixtureDef SensorFixtureDef;
		//	SensorFixtureDef.shape = &sensorShape;
		//	SensorFixtureDef.isSensor = true;	// 設定為 Sensor
		//	SensorFixtureDef.density = 9999 + i; // 故意設定成這個值，方便碰觸時候的判斷
		//	SensorBody->CreateFixture(&SensorFixtureDef);
		//}
		break;
	case 3:
		for (int i = 1; i <= amount; i++)
		{
			ostr.str("");
			ostr << "portal01_0" << i; objname = ostr.str();
			auto portalSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Point loc = portalSprite->getPosition();
			Size size = portalSprite->getContentSize();
			float scaleX = portalSprite->getScaleX();
			float scaleY = portalSprite->getScaleY();
			b2BodyDef portalBodyDef;
			portalBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
			portalBodyDef.type = b2_staticBody;

			b2Body* PortalBody = _b2World->CreateBody(&portalBodyDef);
			b2PolygonShape portalShape;
			portalShape.SetAsBox(size.width * scaleX * 0.5f / PTM_RATIO, size.height * scaleY * 0.5 / PTM_RATIO);

			b2FixtureDef PortalFixtureDef;
			PortalFixtureDef.shape = &portalShape;
			PortalFixtureDef.isSensor = true;
			PortalFixtureDef.density = 9999 + 10 + i;
			PortalBody->CreateFixture(&PortalFixtureDef);

			//_bSparking = true;
		}
		break;
	}

}

void StageThree::createWall()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2Body* body = _b2World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	b2EdgeShape edgeShape;
	fixtureDef.shape = &edgeShape;

	auto wallSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("limitwall"));
	Point loc = wallSprite->getPosition();
	Size size = wallSprite->getContentSize();
	float angle = wallSprite->getRotation();
	float scale = wallSprite->getScaleX();

	Point lep1, lep2, wep1, wep2;
	lep1.y = 0; lep1.x = -(size.width - 4) / 2.0f;
	lep2.y = 0; lep2.x = (size.width - 4) / 2.0f;

	cocos2d::Mat4 modelMatrix, rotMatrix;
	modelMatrix.m[0] = scale;
	cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
	modelMatrix.multiply(rotMatrix);
	modelMatrix.m[3] = _csbRoot->getPosition().x + loc.x;
	modelMatrix.m[7] = _csbRoot->getPosition().y + loc.y;

	wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
	wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
	wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
	wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];

	edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
	fixtureDef.filter.categoryBits = 1 << 1;
	body->CreateFixture(&fixtureDef);
}

void StageThree::reset()
{
	_breset = true;
	Director::getInstance()->getTextureCache()->removeAllTextures();
	test = 0;
	this->removeChild(boxLayer, true);
	boxLayer->release();
	boxLayer = Layer::create();
	this->addChild(boxLayer, 5);

	Sprite* ballData = static_cast<Sprite*>(_player->GetUserData());
	ballData->setPosition(_spawnPoint[0]);
	_player->SetTransform(b2Vec2(_spawnPoint[0].x / PTM_RATIO, _spawnPoint[0].y / PTM_RATIO), 0);

	Sprite* doorData = static_cast<Sprite*>(_door->GetUserData());
	doorData->setPosition(_doorSpawn);
	_door->SetTransform(b2Vec2(_doorSpawn.x / PTM_RATIO, _doorSpawn.y / PTM_RATIO), 0);

	for (int i = 0; i < 5; i++)
	{
		Sprite* ballData = static_cast<Sprite*>(_Rock[i]->GetUserData());
		ballData->setPosition(_spawnPoint[i + 1]);
		_Rock[i]->SetLinearVelocity(b2Vec2(0, 0));
		_Rock[i]->SetTransform(b2Vec2(_spawnPoint[i + 1].x / PTM_RATIO, _spawnPoint[i + 1].y / PTM_RATIO), 0);
	}
}

void StageThree::resetObj()
{
	_resetButton[1]->setVisible(false);
	_returnButton[1]->setVisible(false);

	_contactListener.setGoal(false);
}