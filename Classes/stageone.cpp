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

	// 建立 Box2D world
	_b2World = nullptr;
	b2Vec2 Gravity = b2Vec2(0.0f, -9.8f);		//重力方向
	bool AllowSleep = true;			//允許睡著
	_b2World = new b2World(Gravity);	//創建世界
	_b2World->SetAllowSleeping(AllowSleep);	//設定物件允許睡著

	// Create Scene with csb file
	_csbRoot = CSLoader::createNode("stageone.csb");
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
	_shapeCreator->createShape(1, 10);

	auto listener = EventListenerTouchOneByOne::create();	//創建一個一對一的事件聆聽器
	listener->onTouchBegan = CC_CALLBACK_2(StageOne::onTouchBegan, this);		//加入觸碰開始事件
	listener->onTouchMoved = CC_CALLBACK_2(StageOne::onTouchMoved, this);		//加入觸碰移動事件
	listener->onTouchEnded = CC_CALLBACK_2(StageOne::onTouchEnded, this);		//加入觸碰離開事件

	this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//加入剛創建的事件聆聽器
	this->schedule(CC_SCHEDULE_SELECTOR(StageOne::doStep));

	return true;
}

void StageOne::doStep(float dt)
{
	int velocityIterations = 8;	// 速度迭代次數
	int positionIterations = 1; // 位置迭代次數 迭代次數一般設定為8~10 越高越真實但效率越差
	if (_bToStartScene)
	{
		log("return");
		// 先將這個 SCENE 的 update從 schedule update 中移出
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
			// 改變重力方向
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

#ifdef BOX2D_DEBUG
//改寫繪製方法
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
	// 先建立 ballSprite 的 Sprite 並加入場景中
	auto ballSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName("dount01_66"));
	ballSprite->setScale(0.75f);
	Point loc = ballSprite->getPosition();
	_spawnPoint = ballSprite->getPosition();
	_deafaultColor = ballSprite->getColor();
	float ballScale = ballSprite->getScale();
	//	ballSprite->setPosition(touchLoc);
	//this->addChild(ballSprite, 2);

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
	fixtureDef.restitution = 0.8f;			// 設定恢復係數
	fixtureDef.density = 5.0f;				// 設定密度
	fixtureDef.friction = 0.15f;			// 設定摩擦係數
	ballBody->CreateFixture(&fixtureDef);	// 在 Body 上產生這個剛體的設定
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
			SensorFixtureDef.isSensor = true;	// 設定為 Sensor
			SensorFixtureDef.density = 9999 + i; // 故意設定成這個值，方便碰觸時候的判斷
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
			SensorFixtureDef.isSensor = true;	// 設定為 Sensor
			SensorFixtureDef.density = 9999 + i; // 故意設定成這個值，方便碰觸時候的判斷
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
	b2FixtureDef fixtureDef; // 產生 Fixture
	b2PolygonShape rectShape;
	fixtureDef.shape = &rectShape;

	std::ostringstream ostr;
	std::string objname;

	// 設定三個不同顏色代表三個碰撞測試的群組
	for (int i = 1; i <= 2; i++)
	{
		ostr.str("");
		ostr << "filter1_0" << i; objname = ostr.str();
		// 取得三個設定碰撞過濾器的靜態物體圖示
		auto rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
		bodyDef.userData = rectSprite;
		rectSprite->setColor(filterColor[(i - 1)]);	// 使用 filterColor 已經建立的顏色
		Size ts = rectSprite->getContentSize();
		Point loc = rectSprite->getPosition();
		float scaleX = rectSprite->getScaleX();	// 讀取矩形畫框有對 X 軸縮放
		float scaleY = rectSprite->getScaleY();	// 讀取矩形畫框有對 Y 軸縮放

		bodyDef.position.Set(loc.x / PTM_RATIO, loc.y / PTM_RATIO); // 設定板子所在的位置
		rectbody = _b2World->CreateBody(&bodyDef); // 在 b2World 中建立實體
		float bw = (ts.width - 4) * scaleX;
		float bh = (ts.height - 4) * scaleY;
		// 設定剛體的範圍是一個 BOX （可以縮放成矩形）
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