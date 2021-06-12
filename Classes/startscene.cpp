#include "startscene.h"

USING_NS_CC;


StartScene::StartScene()
{
    _stage = 0;
}

StartScene::~StartScene()
{
    //CC_SAFE_DELETE(_startBtn);
    this->removeAllChildren();
    SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("startscene.plist");
    Director::getInstance()->getTextureCache()->removeUnusedTextures();
}


Scene* StartScene::createScene()
{
    return StartScene::create();
}



// Print useful error message instead of segfaulting when files are not there.
static void problemLoading(const char* filename)
{
    printf("Error while loading: %s\n", filename);
    printf("Depending on how you compiled you might have to add 'Resources/' in front of filenames in HelloWorldScene.cpp\n");
}

// on "init" you need to initialize your instance
bool StartScene::init()
{
    //////////////////////////////
    // 1. super init first
    if (!Scene::init()) { return false; }

    auto visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

    // �]�w�j�M���|
    std::vector<std::string> searchPath;
    searchPath.push_back("Resources/");
    CCFileUtils::getInstance()->setSearchPaths(searchPath);

    // ���J cocos Studio ��X�� csb ��
    auto rootNode = CSLoader::createNode("startscene.csb");
    this->addChild(rootNode); // �[�J�ثe�� scene ��
   
    std::ostringstream ostr;
    std::string objname;

    for (int i = 0; i < STAGE_NUM; i++)
    {
        ostr.str("");
        ostr << "startbtn_0" << (i + 1); objname = ostr.str();
        auto btnloc = dynamic_cast<cocos2d::Sprite*>(rootNode->getChildByName(objname));
        btnloc->setVisible(false);
        _startBtn[i] = new (std::nothrow) CButton();
        _startBtn[i]->setButtonInfo("orange02.png", "orange05.png", btnloc->getPosition());
        _bToGameScene = false;
        this->addChild(_startBtn[i], 10);
    }

    //�Ыؤ@�Ӥ@��@���ƥ��ť��
    auto listener = EventListenerTouchOneByOne::create();
    listener->onTouchBegan = CC_CALLBACK_2(StartScene::onTouchBegan, this);
    listener->onTouchMoved = CC_CALLBACK_2(StartScene::onTouchMoved, this);//�[�JĲ�I���ʨƥ�
    listener->onTouchEnded = CC_CALLBACK_2(StartScene::onTouchEnded, this);//�[�JĲ�I���}�ƥ�

    this->_eventDispatcher->addEventListenerWithSceneGraphPriority(listener, this);	//�[�J��Ыت��ƥ��ť��
    this->schedule(CC_SCHEDULE_SELECTOR(StartScene::update));

    return true;
}



void StartScene::update(float dt)
{
    if (_bToGameScene) { // ������ SecondScene
        // ���N�o�� SCENE �� update  �q schedule update �����X       
        this->unschedule(schedule_selector(StartScene::update));
        SpriteFrameCache::getInstance()->removeSpriteFramesFromFile("startscene.plist");
        //  �]�w�����������S��
        //TransitionFade *pageTurn = TransitionFade::create(1.0F, GameScene::createScene());
        //Director::getInstance()->replaceScene(pageTurn);
        switch (_stage)
        {
        case 1:
            Director::getInstance()->replaceScene(StageOne::createScene());
            break;
        case 2:
            Director::getInstance()->replaceScene(StageTwo::createScene());
            break;
        case 3:
            Director::getInstance()->replaceScene(StageThree::createScene());
            break;
        case 4:
            Director::getInstance()->replaceScene(StageFour::createScene());
            break;
        }
        
        // �����n���g�b�o��
    }
}

bool StartScene::onTouchBegan(cocos2d::Touch* pTouch, cocos2d::Event* pEvent)//Ĳ�I�}�l�ƥ�
{
    Point touchLoc = pTouch->getLocation();
    _startBtn[0]->touchesBegin(touchLoc);
    _startBtn[1]->touchesBegin(touchLoc);
    _startBtn[2]->touchesBegin(touchLoc);
    _startBtn[3]->touchesBegin(touchLoc);
    return true;
}


void StartScene::onTouchMoved(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I���ʨƥ�
{
    Point touchLoc = pTouch->getLocation();
    _startBtn[0]->touchesMoved(touchLoc);
    _startBtn[1]->touchesMoved(touchLoc);
    _startBtn[2]->touchesMoved(touchLoc);
    _startBtn[3]->touchesMoved(touchLoc);
}

void  StartScene::onTouchEnded(cocos2d::Touch* pTouch, cocos2d::Event* pEvent) //Ĳ�I�����ƥ� 
{
    Point touchLoc = pTouch->getLocation();
    if (_startBtn[0]->touchesEnded(touchLoc)) { // �i�����������
        _bToGameScene = true;
        _stage = 1;
    }
    else if (_startBtn[1]->touchesEnded(touchLoc))
    {
        _bToGameScene = true;
        _stage = 2;
    }
    else if (_startBtn[2]->touchesEnded(touchLoc))
    {
        _bToGameScene = true;
        _stage = 3;
    }
    else if (_startBtn[3]->touchesEnded(touchLoc))
    {
        _bToGameScene = true;
        _stage = 4;
    }    
}