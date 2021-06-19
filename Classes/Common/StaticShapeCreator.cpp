#include "StaticShapeCreator.h"
#include "cocostudio/CocoStudio.h"

USING_NS_CC;

void StaticShapeCreator::init(b2World& world, Size& visible, cocos2d::Scene& theScene, cocos2d::Node& csb)
{
	_World = &world;
	_visibleSize = &visible;
	_Scene = &theScene;

	_csbRoot = &csb;
	_fatherPnt = _csbRoot->getPosition();
}

void StaticShapeCreator::createShape(int type, int amount)
{
	b2BodyDef bodyDef;
	bodyDef.type = b2_staticBody;
	bodyDef.userData = NULL;
	b2Body* body = _World->CreateBody(&bodyDef);

	b2FixtureDef fixtureDef;
	b2EdgeShape edgeShape;
	b2PolygonShape rectShape;
	b2PolygonShape triShape;
	b2CircleShape sphereShape;
	switch (type)
	{
	case EDGE:		
		fixtureDef.shape = &edgeShape;
		for (size_t i = 1; i <= amount; i++)
		{
			std::ostringstream ostr;
			std::string objname;
			ostr << "block1_0" << i; objname = ostr.str();

			auto const edgeSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Size ts = edgeSprite->getContentSize();
			Point loc = edgeSprite->getPosition();
			float angle = edgeSprite->getRotation();
			float scale = edgeSprite->getScaleX();

			Point lep1, lep2, wep1, wep2;
			lep1.y = 0; lep1.x = -(ts.width - 4) / 2.0f;
			lep2.y = 0; lep2.x = (ts.width - 4) / 2.0f;

			cocos2d::Mat4 modelMatrix, rotMatrix;
			modelMatrix.m[0] = scale;
			cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
			modelMatrix.multiply(rotMatrix);
			modelMatrix.m[3] = _fatherPnt.x + loc.x;
			modelMatrix.m[7] = _fatherPnt.y + loc.y;

			wep1.x = lep1.x * modelMatrix.m[0] + lep1.y * modelMatrix.m[1] + modelMatrix.m[3];
			wep1.y = lep1.x * modelMatrix.m[4] + lep1.y * modelMatrix.m[5] + modelMatrix.m[7];
			wep2.x = lep2.x * modelMatrix.m[0] + lep2.y * modelMatrix.m[1] + modelMatrix.m[3];
			wep2.y = lep2.x * modelMatrix.m[4] + lep2.y * modelMatrix.m[5] + modelMatrix.m[7];
			
			edgeShape.Set(b2Vec2(wep1.x / PTM_RATIO, wep1.y / PTM_RATIO), b2Vec2(wep2.x / PTM_RATIO, wep2.y / PTM_RATIO));
			body->CreateFixture(&fixtureDef);
		}
		break;
	case RECT:		
		fixtureDef.shape = &rectShape;
		for (size_t i = 1; i < amount; i++)
		{
			std::ostringstream ostr;
			std::string objname;
			ostr << "ployblock1_0" << i; objname = ostr.str();

			auto const rectSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Size ts = rectSprite->getContentSize();
			Point loc = rectSprite->getPosition();
			float angle = rectSprite->getRotation();
			float scaleX = rectSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
			float scaleY = rectSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

			// rectShape 的四個端點, 0 右上、 1 左上、 2 左下 3 右下
			Point lep[4], wep[4];
			lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
			lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
			lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
			lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

			// 所有的線段圖示都是是本身的中心點為 (0,0)，
			// 根據縮放、旋轉產生所需要的矩陣
			// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
			// 然後進行旋轉，
			// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
			cocos2d::Mat4 modelMatrix, rotMatrix;
			modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
			modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
			cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
			modelMatrix.multiply(rotMatrix);
			modelMatrix.m[3] = _fatherPnt.x + loc.x; //設定 Translation，自己的加上父親的
			modelMatrix.m[7] = _fatherPnt.y + loc.y; //設定 Translation，自己的加上父親的
			for (size_t j = 0; j < 4; j++)
			{
				wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
				wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
			}
			b2Vec2 vecs[] = {
				b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
				b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
				b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO),
				b2Vec2(wep[3].x / PTM_RATIO, wep[3].y / PTM_RATIO) };

			rectShape.Set(vecs, 4);
			body->CreateFixture(&fixtureDef);
		}
		break;
	case TRIANGLE:
		fixtureDef.shape = &triShape;
		for (size_t i = 1; i < amount; i++)
		{
			std::ostringstream ostr;
			std::string objname;
			ostr << "triangle1_0" << i; objname = ostr.str();

			const auto triSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Size ts = triSprite->getContentSize();
			Point loc = triSprite->getPosition();
			float angle = triSprite->getRotation();
			float scaleX = triSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
			float scaleY = triSprite->getScaleY();	// 水平的線段圖示假設都只有對 X 軸放大

			Point lep[3], wep[3];	// triShape 的三個頂點, 0 頂點、 1 左下、 2 右下
			lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
			lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
			lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


			// 所有的線段圖示都是是本身的中心點為 (0,0)，
			// 根據縮放、旋轉產生所需要的矩陣
			// 根據寬度計算出兩個端點的座標，然後呈上開矩陣
			// 然後進行旋轉，
			// Step1: 先CHECK 有無旋轉，有旋轉則進行端點的計算
			cocos2d::Mat4 modelMatrix, rotMatrix;
			modelMatrix.m[0] = scaleX;  // 先設定 X 軸的縮放
			modelMatrix.m[5] = scaleY;  // 先設定 Y 軸的縮放
			cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
			modelMatrix.multiply(rotMatrix);
			modelMatrix.m[3] = _fatherPnt.x + loc.x; //設定 Translation，自己的加上父親的
			modelMatrix.m[7] = _fatherPnt.y + loc.y; //設定 Translation，自己的加上父親的
			for (size_t j = 0; j < 3; j++)
			{
				wep[j].x = lep[j].x * modelMatrix.m[0] + lep[j].y * modelMatrix.m[1] + modelMatrix.m[3];
				wep[j].y = lep[j].x * modelMatrix.m[4] + lep[j].y * modelMatrix.m[5] + modelMatrix.m[7];
			}
			b2Vec2 vecs[] = {
				b2Vec2(wep[0].x / PTM_RATIO, wep[0].y / PTM_RATIO),
				b2Vec2(wep[1].x / PTM_RATIO, wep[1].y / PTM_RATIO),
				b2Vec2(wep[2].x / PTM_RATIO, wep[2].y / PTM_RATIO) };

			triShape.Set(vecs, 3);
			body->CreateFixture(&fixtureDef);
		}
		break;
	case SPHERE:
		fixtureDef.shape = &sphereShape;
		for (size_t i = 1; i < amount; i++)
		{
			std::ostringstream ostr;
			std::string objname;
			ostr << "ball1_0" << i; objname = ostr.str();

			const auto circleSprite = dynamic_cast<Sprite*>(_csbRoot->getChildByName(objname));
			Size ts = circleSprite->getContentSize();
			Point loc = circleSprite->getPosition();
			float scaleX = circleSprite->getScaleX();	// 水平的線段圖示假設都只有對 X 軸放大
			float radius = (ts.width - 4) * scaleX * 0.5f;

			Point wloc;
			wloc.x = loc.x + _fatherPnt.x; wloc.y = loc.y + _fatherPnt.y;
			sphereShape.m_radius = radius / PTM_RATIO;
			bodyDef.position.Set(wloc.x / PTM_RATIO, wloc.y / PTM_RATIO);
			//b2Body* body = _World->CreateBody(&bodyDef);
			body->CreateFixture(&fixtureDef);
		}
		break;
	}
}