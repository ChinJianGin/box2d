#include "stagefour.h"
#include "cocostudio/CocoStudio.h"
#include "ui/CocosGUI.h"

USING_NS_CC;
using namespace cocostudio::timeline;

#define MAX_2(X,Y) (X)>(Y) ? (X) : (Y)

StageFour::~StageFour()
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

Scene* StageFour::createScene()
{
	return StageFour::create();
}

bool StageFour::init()
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

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	//b2Vec2 Gravity = b2Vec2(9.8f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("stagefour.csb");
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

	btnSprite = dynamic_cast<Sprite*>(_endNode->getChildByName("resetbtn"));
	_resetBtn = CButton::create();
	_resetBtn->setButtonInfo("replaybtn.png", "replaybtn.png", btnSprite->getPosition());
	_resetBtn->setScale(btnSprite->getScale());
	_resetBtn->setVisible(false);
	this->addChild(_resetBtn, 3);
	btnSprite->setVisible(false);

	createStaticBoundary();
	createSensor(1, 0);
	createSensor(4, 3);
	createCar();
	createBridgeAndRope();

	_bTouchOn = false;

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
	_shapeCreator->createShape(2, 2);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(StageFour::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(StageFour::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(StageFour::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(StageFour::doStep));

	return true;
}

void StageFour::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	if (_bToStartScene)
	{
		log("return");
		// 先將這個 SCENE 的 update從 schedule update 中移出
		this->unschedule(schedule_selector(StageFour::doStep));
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
			// 以下是以 Body 有包含 Sprite 顯示為例
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

bool StageFour::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	log("stage four return btn");
	for (int i = 0; i < 2; i++)
	{
		_returnButton[i]->touchesBegin(touchLoc);
	}
	_resetBtn->touchesBegin(touchLoc);

	// For Mouse Joiint 
	for (b2Body* body = _b2World->GetBodyList(); body; body = body->GetNext())
	{
		if (body->GetUserData() == NULL) continue; // 靜態物體不處理
		// 判斷點的位置是否落在動態物體一定的範圍
		Sprite* spriteObj = static_cast<Sprite*>(body->GetUserData());
		Size objSize = spriteObj->getContentSize();
		float fdist = MAX_2(objSize.width, objSize.height) / 2.0f;
		float x = body->GetPosition().x * PTM_RATIO - touchLoc.x;
		float y = body->GetPosition().y * PTM_RATIO - touchLoc.y;
		float tpdist = x * x + y * y;
		if (tpdist < fdist * fdist)
		{
			_bTouchOn = true;
			b2MouseJointDef mouseJointDef;
			mouseJointDef.bodyA = _bottomBody;
			mouseJointDef.bodyB = body;
			mouseJointDef.target = b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO);
			mouseJointDef.collideConnected = true;
			mouseJointDef.maxForce = 1000.0f * body->GetMass();
			_MouseJoint = dynamic_cast<b2MouseJoint*>(_b2World->CreateJoint(&mouseJointDef)); // 新增 Mouse Joint
			body->SetAwake(true);
			break;
		}
	}
	return true;
}

void StageFour::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	for (int i = 0; i < 2; i++)
	{
		_returnButton[i]->touchesMoved(touchLoc);
	}
	_resetBtn->touchesMoved(touchLoc);
	if (_bTouchOn)
	{
		_MouseJoint->SetTarget(b2Vec2(touchLoc.x / PTM_RATIO, touchLoc.y / PTM_RATIO));
	}
}

void StageFour::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)
{
	Point touchLoc = pTouch->getLocation();
	for (int i = 0; i < 2; i++)
	{
		if (_returnButton[i]->touchesEnded(touchLoc))
		{
			_bToStartScene = true;
		}
	}
	if (_resetBtn->touchesEnded(touchLoc))
	{
		reset();
	}
	if (_bTouchOn)
	{
		_bTouchOn = false;
		if (_MouseJoint != NULL)
		{
			_b2World->DestroyJoint(_MouseJoint);
			_MouseJoint = NULL;
		}
	}
}

#ifdef BOX2D_DEBUG
//改寫繪製方法
void StageFour::draw(Renderer* renderer, const Mat4& transform, uint32_t flags)
{
	Director* director = Director::getInstance();

	GL::enableVertexAttribs(cocos2d::GL::VERTEX_ATTRIB_FLAG_POSITION);
	director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
	_b2World->DrawDebugData();
	director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
}
#endif

void StageFour::createStaticBoundary()
{
	// 先產生 Body, 設定相關的參數

	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody; // 設定這個 Body 為 靜態的
	bodyDef.userData = NULL;
	// 在 b2World 中產生該 Body, 並傳回產生的 b2Body 物件的指標
	// 產生一次，就可以讓後面所有的 Shape 使用
	b2Body* body = _b2World->CreateBody(&bodyDef);

	_bottomBody = body;

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

void StageFour::createSensor(int type, int amount)
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
	case 4:
		for (int i = 1; i <= amount; i++)
		{
			ostr.str("");
			ostr << "force0" << i; objname = ostr.str();
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
			portalShape.SetAsBox((size.width - 4) * scaleX * 0.5f / PTM_RATIO, (size.height - 4) * scaleY * 0.5 / PTM_RATIO);

			b2FixtureDef PortalFixtureDef;
			PortalFixtureDef.shape = &portalShape;
			PortalFixtureDef.isSensor = true;
			PortalFixtureDef.density = 9999 + 20 + i;
			PortalBody->CreateFixture(&PortalFixtureDef);

			//_bSparking = true;
		}
		break;
	}

}

void StageFour::createCar()
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
	//_rearWheel->SetAngularVelocity(-100.0f);

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

void StageFour::createBridgeAndRope()
{
	
	Sprite* gearSprite[4] = { nullptr };
	Point loc[4];
	Size size[4];
	float scale[4] = { 0 };
	b2Body* staticBody[4] = { nullptr };
	b2Body* dynamicBody[4] = { nullptr };
	b2RevoluteJoint* RevJoint[2] = { nullptr };
	b2PrismaticJoint* PriJoint[2] = { nullptr };

	b2BodyDef staticBodyDef;
	staticBodyDef.type = b2_staticBody;
	staticBodyDef.userData = NULL;

	b2CircleShape staticShape;
	staticShape.m_radius = 5 / PTM_RATIO;
	b2FixtureDef fixtureDef;
	fixtureDef.shape = &staticShape;

	for (int i = 0; i < 4; i++)
	{
		std::ostringstream ostr;
		std::string objname;
		ostr << "gear_0" << i + 1; objname = ostr.str();

		gearSprite[i] = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		loc[i] = gearSprite[i]->getPosition();
		size[i] = gearSprite[i]->getContentSize();
		scale[i] = gearSprite[i]->getScale();

		staticBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		staticBody[i] = _b2World->CreateBody(&staticBodyDef);
		staticBody[i]->CreateFixture(&fixtureDef);
	}
	b2BodyDef dynamicBodyDef;
	dynamicBodyDef.type = b2_dynamicBody;

	b2CircleShape circleShape;
	b2PolygonShape polyShape;
	fixtureDef.shape = &circleShape;
	fixtureDef.density = 1.0f;
	fixtureDef.friction = 0.2f;
	fixtureDef.restitution = 0.25f;

	for (int i = 0; i < 4; i++)
	{
		if (i < 3)circleShape.m_radius = (size[i].width - 4) * 0.5f * scale[i] / PTM_RATIO;
		else
		{
			float sx = gearSprite[i]->getScaleX();
			float sy = gearSprite[i]->getScaleY();
			fixtureDef.shape = &polyShape;
			fixtureDef.filter.categoryBits = 1 << 1;
			polyShape.SetAsBox((size[i].width - 4) * 0.5f * sx / PTM_RATIO, (size[i].height - 4) * 0.5f * sy / PTM_RATIO);
		}
		if (i == 0)fixtureDef.filter.maskBits = 1 << 1 | 1;
		dynamicBodyDef.userData = gearSprite[i];
		dynamicBodyDef.position.Set(loc[i].x / PTM_RATIO, loc[i].y / PTM_RATIO);
		dynamicBody[i] = _b2World->CreateBody(&dynamicBodyDef);
		dynamicBody[i]->CreateFixture(&fixtureDef);
	}

	b2RevoluteJointDef RJoint;
	b2PrismaticJointDef PrJoint;
	for (int i = 0; i < 4; i++)
	{
		if (i > 0 && i < 3)
		{
			RJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter());
			RevJoint[i - 1] = dynamic_cast<b2RevoluteJoint*>(_b2World->CreateJoint(&RJoint));
		}
		else if(i == 0)
		{
			PrJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter(), b2Vec2(0, -1.0f));
			PriJoint[0] = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&PrJoint));
		}
		else
		{
			PrJoint.Initialize(staticBody[i], dynamicBody[i], dynamicBody[i]->GetWorldCenter(), b2Vec2(1.0f, 0));
			PriJoint[1] = dynamic_cast<b2PrismaticJoint*>(_b2World->CreateJoint(&PrJoint));
		}
	}

	b2GearJointDef GJoint;
	GJoint.bodyA = dynamicBody[1];
	GJoint.bodyB = dynamicBody[2];
	GJoint.joint1 = RevJoint[0];
	GJoint.joint2 = RevJoint[1];
	GJoint.ratio = -1;
	_b2World->CreateJoint(&GJoint);

	GJoint.bodyA = dynamicBody[2];
	GJoint.bodyB = dynamicBody[3];
	GJoint.joint1 = RevJoint[1];
	GJoint.joint2 = PriJoint[1];
	GJoint.ratio = 1;
	_b2World->CreateJoint(&GJoint);

	auto bridgeSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("bridge"));
	Point bridgeLoc = bridgeSprite->getPosition();
	Size bridgeSize = bridgeSprite->getContentSize();
	float scaleX = bridgeSprite->getScaleX();
	float scaleY = bridgeSprite->getScaleY();

	dynamicBodyDef.position.Set(bridgeLoc.x / PTM_RATIO, bridgeLoc.y / PTM_RATIO);
	dynamicBodyDef.userData = bridgeSprite;
	b2Body* bridgeBody = _b2World->CreateBody(&dynamicBodyDef);

	b2PolygonShape bridgeShape;
	bridgeShape.SetAsBox((bridgeSize.width - 4) * scaleX * 0.5f / PTM_RATIO, (bridgeSize.height - 4) * scaleY * 0.5f / PTM_RATIO);

	fixtureDef.shape = &bridgeShape;
	fixtureDef.density = 7.0f;
	fixtureDef.friction = 0.1f;
	fixtureDef.restitution = 0;
	fixtureDef.filter.maskBits = 1 << 2 | 1;
	fixtureDef.filter.categoryBits = 1 << 1;

	bridgeBody->CreateFixture(&fixtureDef);


}

void StageFour::reset()
{
	_returnButton[1]->setVisible(false);
	_resetBtn->setVisible(false);
}