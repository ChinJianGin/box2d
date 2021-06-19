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
			float scaleX = rectSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
			float scaleY = rectSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

			// rectShape ���|�Ӻ��I, 0 �k�W�B 1 ���W�B 2 ���U 3 �k�U
			Point lep[4], wep[4];
			lep[0].x = (ts.width - 4) / 2.0f;;  lep[0].y = (ts.height - 4) / 2.0f;
			lep[1].x = -(ts.width - 4) / 2.0f;; lep[1].y = (ts.height - 4) / 2.0f;
			lep[2].x = -(ts.width - 4) / 2.0f;; lep[2].y = -(ts.height - 4) / 2.0f;
			lep[3].x = (ts.width - 4) / 2.0f;;  lep[3].y = -(ts.height - 4) / 2.0f;

			// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
			// �ھ��Y��B���ಣ�ͩһݭn���x�}
			// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
			// �M��i�����A
			// Step1: ��CHECK ���L����A������h�i����I���p��
			cocos2d::Mat4 modelMatrix, rotMatrix;
			modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
			modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
			cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
			modelMatrix.multiply(rotMatrix);
			modelMatrix.m[3] = _fatherPnt.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
			modelMatrix.m[7] = _fatherPnt.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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
			float scaleX = triSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
			float scaleY = triSprite->getScaleY();	// �������u�q�ϥܰ��]���u���� X �b��j

			Point lep[3], wep[3];	// triShape ���T�ӳ��I, 0 ���I�B 1 ���U�B 2 �k�U
			lep[0].x = 0;  lep[0].y = (ts.height - 2) / 2.0f;
			lep[1].x = -(ts.width - 2) / 2.0f; lep[1].y = -(ts.height - 2) / 2.0f;
			lep[2].x = (ts.width - 2) / 2.0f; lep[2].y = -(ts.height - 2) / 2.0f;


			// �Ҧ����u�q�ϥܳ��O�O�����������I�� (0,0)�A
			// �ھ��Y��B���ಣ�ͩһݭn���x�}
			// �ھڼe�׭p��X��Ӻ��I���y�СA�M��e�W�}�x�}
			// �M��i�����A
			// Step1: ��CHECK ���L����A������h�i����I���p��
			cocos2d::Mat4 modelMatrix, rotMatrix;
			modelMatrix.m[0] = scaleX;  // ���]�w X �b���Y��
			modelMatrix.m[5] = scaleY;  // ���]�w Y �b���Y��
			cocos2d::Mat4::createRotationZ(angle * M_PI / 180.0f, &rotMatrix);
			modelMatrix.multiply(rotMatrix);
			modelMatrix.m[3] = _fatherPnt.x + loc.x; //�]�w Translation�A�ۤv���[�W���˪�
			modelMatrix.m[7] = _fatherPnt.y + loc.y; //�]�w Translation�A�ۤv���[�W���˪�
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
			float scaleX = circleSprite->getScaleX();	// �������u�q�ϥܰ��]���u���� X �b��j
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