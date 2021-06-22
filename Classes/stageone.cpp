#include "stageone.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

#ifndef MAX_CIRCLE_OBJECTS_1
#define MAX_CIRCLE_OBJECTS_1  11
#endif

char g_CircleObject_1[MAX_CIRCLE_OBJECTS_1][20] = {
	"clock01.png","clock02.png","clock03.png","clock04.png",
	"dount01.png","dount02.png","dount03.png","dount04.png",
	"orange01.png","orange02.png","orange03.png" };

USING_NS_CC;
using namespace cocostudio::timeline;

StageOne::~StageOne()
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

Scene* StageOne::createScene()
{
	return StageOne::create();
}

bool StageOne::init()
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
	_csbRoot = CSLoader::createNode("stageone.csb");
	_endNode = CSLoader::createNode("endNode.csb");
#ifndef BOX2D_DEBUG
	// �]�w��ܭI���ϥ�
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
	this->addChild(_returnButton[1], 10);
	btnSprite->setVisible(false);

	btnSprite = dynamic_cast<Sprite*>(_endNode->getChildByName("resetbtn"));
	_resetBtn = CButton::create();
	_resetBtn->setButtonInfo("replaybtn.png", "replaybtn.png", btnSprite->getPosition());
	_resetBtn->setScale(btnSprite->getScale());
	_resetBtn->setVisible(false);
	this->addChild(_resetBtn, 10);
	btnSprite->setVisible(false);



	createStaticBoundary();
	setGravityButton();
	createPlayer();
	createFilter();
	createSensor(1 , 0);
	createSensor(2 , 2);

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

	_b2World->SetContactListener(&_contactListener);

	_shapeCreator = new StaticShapeCreator;
	_shapeCreator->init(*_b2World, _visibleSize, *this, *_csbRoot);
	_shapeCreator->createShape(1, 10);

	auto listener = EventListenerTouchOneByOne::create();	//�Ыؤ@�Ӥ@��@���ƥ��ť��
	listener->onTouchBegan = CC_CALLBACK_2(StageOne::onTouchBegan, this);		//�[�JĲ�I�}�l�ƥ�
	listener->onTouchMoved = CC_CALLBACK_2(StageOne::onTouchMoved, this);		//�[�JĲ�I���ʨƥ�
	listener->onTouchEnded = CC_CALLBACK_2(StageOne::onTouchEnded, this);		//�[�JĲ�I���}�ƥ�

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
	this->schedule(CC_SCHEDULE_SELECTOR(StageOne::doStep));

	return true;
}

void StageOne::doStep(float dt)
{
	int velocityIterations = 8;	// �t�׭��N����
	int positionIterations = 1; // ��m���N���� ���N���Ƥ@��]�w��8~10 �V���V�u����Ĳv�V�t
	if (_bToStartScene)
	{
		log("return");
		// ���N�o�� SCENE �� update�q schedule update �����X
		this->unschedule(schedule_selector(StageOne::doStep));
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("gamescene.plist");
		SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("box2d.plist");
		TransitionFade* pageTurn = TransitionFade::create(1.0F, StartScene::createScene());
		Director::getInstance()->replaceScene(pageTurn);
	}
	if (!_contactListener.isGoal())
	{
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
	Sprite* ballData = static_cast<Sprite*>(_player->GetUserData());
	if (_contactListener.isGoal())
	{		
		_returnButton[1]->setVisible(true);
		_resetBtn->setVisible(true);
	}
}

bool StageOne::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	bool bOnGravityBtn = false;
	for (size_t i = 0; i < 4; i++)
	{
		if (_gravityBtn[i]->touchesBegin(touchLoc)) {
			bOnGravityBtn = true;
			break;
		}
	}
	log("stage one return btn");
	if (_returnButton[0]->touchesBegin(touchLoc) || _returnButton[1]->touchesBegin(touchLoc))
	{
		
	}
	if (_resetBtn->touchesBegin(touchLoc))
	{

	}
	return true;
}

void StageOne::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	_returnButton[0]->touchesMoved(touchLoc);
	_returnButton[1]->touchesMoved(touchLoc);
	_resetBtn->touchesMoved(touchLoc);
	for (size_t i = 0; i < 4; i++)
	{
		if (_gravityBtn[i]->touchesMoved(touchLoc)) {
			break;
		}
	}
}

void StageOne::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	if (_returnButton[0]->touchesEnded(touchLoc) || _returnButton[1]->touchesEnded(touchLoc))
	{
		_bToStartScene = true;
	}
	for (size_t i = 0; i < 4; i++)
	{
		if (_gravityBtn[i]->touchesEnded(touchLoc)) {
			// ���ܭ��O��V
			if (i == 0) _b2World->SetGravity(b2Vec2(0, -9.8f));
			else if (i == 1) _b2World->SetGravity(b2Vec2(-9.8f, 0));
			else if (i == 2) _b2World->SetGravity(b2Vec2(0, 9.8f));
			else  _b2World->SetGravity(b2Vec2(9.8f, 0));
			break;
		}
	}
	if (_resetBtn->touchesEnded(touchLoc))
	{
		reset();
	}
}

void StageOne::createStaticBoundary()
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

#ifdef BOX2D_DEBUG
//��gø�s��k
void StageOne::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

void StageOne::setGravityButton() {
	_gravityBtn[0] = CButton::create();
	_gravityBtn[0]->setButtonInfo("dnarrow.png", "dnarrowon.png", Point(_visibleSize.width / 2.0f, 50.0f));
	_gravityBtn[0]->setScale(0.75f);
	this->addChild(_gravityBtn[0], 10);

	_gravityBtn[1] = CButton::create();
	_gravityBtn[1]->setButtonInfo("leftarrow.png", "leftarrowon.png", Point(50.0f, _visibleSize.height / 2.0f));
	_gravityBtn[1]->setScale(0.75f);
	this->addChild(_gravityBtn[1], 10);

	_gravityBtn[2] = CButton::create();
	_gravityBtn[2]->setButtonInfo("uparrow.png", "uparrowon.png", Point(_visibleSize.width / 2.0f, _visibleSize.height - 50.0f));
	_gravityBtn[2]->setScale(0.75f);
	this->addChild(_gravityBtn[2], 10);

	_gravityBtn[3] = CButton::create();
	_gravityBtn[3]->setButtonInfo("rightarrow.png", "rightarrowon.png", Point(_visibleSize.width - 50.0f, _visibleSize.height / 2.0f));
	_gravityBtn[3]->setScale(0.75f);
	this->addChild(_gravityBtn[3], 10);
}

void StageOne::createPlayer()
{
	// ���إ� ballSprite �� Sprite �å[�J������
	auto ballSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("dount01_66"));
	ballSprite->setScale(0.75f);
	Point loc = ballSprite->getPosition();
	_spawnPoint = ballSprite->getPosition();
	_deafaultColor = ballSprite->getColor();
	float ballScale = ballSprite->getScale();
	//	ballSprite->setPosition(touchLoc);
	//this->addChild(ballSprite, 2);

	// �إߤ@��²�檺�ʺA�y��
	b2BodyDef bodyDef;	// ���H���c b2BodyDef �ŧi�@�� Body ���ܼ�
	bodyDef.type = b2_dynamicBody; // �]�w���ʺA����
	bodyDef.userData = ballSprite;	// �]�w Sprite ���ʺA���骺��ܹϥ�
	bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
	// �H bodyDef �b b2World  ���إ߹���öǦ^�ӹ��骺����
	b2Body* ballBody = _b2World->CreateBody(&bodyDef);
	// �]�w�Ӫ��骺�~��
	b2CircleShape ballShape;	//  �ŧi���骺�~�������ܼơA���B�O��Ϊ���
	Size ballsize = ballSprite->getContentSize();	// �ھ� Sprite �ϧΪ��j�p�ӳ]�w��Ϊ��b�|
	ballShape.m_radius = (ballsize.width * ballScale) * 0.5f / PTM_RATIO;
	// �H b2FixtureDef  ���c�ŧi���鵲�c�ܼơA�ó]�w���骺�������z�Y��
	b2FixtureDef fixtureDef;	 // �T�w�˸m
	fixtureDef.shape = &ballShape;			// ���w���骺�~�������
	fixtureDef.restitution = 0.8f;			// �]�w��_�Y��
	fixtureDef.density = 5.0f;				// �]�w�K��
	fixtureDef.friction = 0.15f;			// �]�w�����Y��
	ballBody->CreateFixture(&fixtureDef);	// �b Body �W���ͳo�ӭ��骺�]�w
	_player = ballBody;
}

void StageOne::createSensor(int type, int amount)
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
			SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
			SensorFixtureDef.density = 9999 + i; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
			SensorBody->CreateFixture(&SensorFixtureDef);
		}
		break;
	case 2:
		for (int i = 2; i <= amount + 1; i++)
		{
			ostr.str("");
			ostr << "sensor0" << i; objname = ostr.str();
			auto sensorSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Point loc = sensorSprite->getPosition();
			Size  size = sensorSprite->getContentSize();
			float scale = sensorSprite->getScale();
			sensorSprite->setColor(filterColor[i -2]);
			b2BodyDef sensorBodyDef;
			sensorBodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO);
			sensorBodyDef.type = b2_staticBody;

			b2Body* SensorBody = _b2World->CreateBody(&sensorBodyDef);
			b2PolygonShape sensorShape;
			sensorShape.SetAsBox(size.width * 0.5f * scale / PTM_RATIO, size.height * 0.5f * scale / PTM_RATIO);

			b2FixtureDef SensorFixtureDef;
			SensorFixtureDef.shape = &sensorShape;
			SensorFixtureDef.isSensor = true;	// �]�w�� Sensor
			SensorFixtureDef.density = 9999 + i; // �G�N�]�w���o�ӭȡA��K�IĲ�ɭԪ��P�_
			SensorBody->CreateFixture(&SensorFixtureDef);
		}
		break;
	}

}

void StageOne::createFilter()
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	b2Body* rectbody;
	b2FixtureDef fixtureDef; // ���� Fixture
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	std::ostringstream ostr;
	std::string objname;

	// �]�w�T�Ӥ��P�C��N��T�ӸI�����ժ��s��
	for (int i = 1; i <= 2; i++)
	{
		ostr.str("");
		ostr << "filter1_0" << i; objname = ostr.str();
		// ���o�T�ӳ]�w�I���L�o�����R�A����ϥ�
		auto rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		bodyDef.userData = rectSprite;
		rectSprite->setColor(filterColor[(i - 1)]);	// �ϥ� filterColor �w�g�إߪ��C��
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float scaleX = rectSprite->getScaleX();	// Ū���x�εe�ئ��� X �b�Y��
		float scaleY = rectSprite->getScaleY();	// Ū���x�εe�ئ��� Y �b�Y��

		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO); // �]�w�O�l�Ҧb����m
		rectbody = _b2World->CreateBody(&bodyDef); // �b b2World ���إ߹���
		float bw = (ts.width - 4) * scaleX;
		float bh = (ts.height - 4) * scaleY;
		// �]�w���骺�d��O�@�� BOX �]�i�H�Y�񦨯x�Ρ^
		rectShape.SetAsBox(bw * 0.5f / PTM_RATIO, bh * 0.5f / PTM_RATIO);
		fixtureDef.filter.categoryBits = 1 << i;
		rectbody->CreateFixture(&fixtureDef);
	}
}

void StageOne::reset()
{
	b2Filter filter;
	_resetBtn->setVisible(false);
	_returnButton[1]->setVisible(false);
	_b2World->SetGravity(b2Vec2(0, -9.8f));
	Sprite* ballData = static_cast<Sprite*>(_player->GetUserData());
	ballData->setPosition(_spawnPoint);
	ballData->setColor(_deafaultColor);
	filter.maskBits = 1 << 1 | 1;
	_player->GetFixtureList()->SetFilterData(filter);
	_player->SetTransform(b2Vec2(_spawnPoint.x / PTM_RATIO, _spawnPoint.y / PTM_RATIO) , 0);
	_contactListener.setGoal(false);
}