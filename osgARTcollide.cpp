/*
 *
 * simple example demonstrate interaction with two markers.
 * 
 * when the two markers are closed, the models turn upside down.
 * (proximity based interaction).
 *
 * usage: osgARTcolllide.exe model_name1 size_model shift_x shift_y shift_z model_name2 size_model shift_x shift_y shift_z"<<std::endl;
 *
 *
 * Copyright (c) 2005-2006
 * Julian Looser, Philip Lamb, Rapha‘l Grasset, Hartmut Seichter.
 *
 */

#include <Producer/RenderSurface>
#include <osgProducer/Viewer>

#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/Projection>
#include <osg/AutoTransform>
#include <osg/ShapeDrawable>
#include <osg/Geometry>

#include <osgDB/ReadFile>

#include <osg/LightModel>
#include <osg/LightSource>

#include <osgART/Foundation>
#include <osgART/VideoManager>
#include <osgART/ARTTransform>
#include <osgART/TrackerManager>
#include <osgART/VideoBackground>
#include <osgART/VideoPlane>
#include <osgART/VideoForeground>

#include "Windows.h"
#include "MMSystem.h"
osg::Node* addLight(int markerId,osg::StateSet* rootStateSet)
{
    osg::Group* lightGroup = new osg::Group;
   
    // create a directional light (infinite distance place at 45 degrees)
    osg::Light* myLight = new osg::Light;
    myLight->setLightNum(1);
	if (markerId)
	{	
		myLight->setPosition(osg::Vec4(-1.0,1.0,-1.0,0.0f));
	}
	else
	{
		myLight->setPosition(osg::Vec4(1.0,-1.0,-1.0,0.0f));
	}
    myLight->setAmbient(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    myLight->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    myLight->setConstantAttenuation(2.0f);

    osg::LightSource* lightS = new osg::LightSource;	
    lightS->setLight(myLight);
    lightS->setLocalStateSetModes(osg::StateAttribute::ON); 

    lightS->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
 
    lightGroup->addChild(lightS);

    return lightGroup;
}

osg::Node* addARTModel(osg::MatrixTransform* interactionTransform, unsigned int markerId,char* name,float sizeModel,float shiftX, float shiftY, float heightMarker)
{

	// create a new node by reading in model from file
	osg::Node*	modelNode = osgDB::readNodeFile(name);

	//put model in origin
	osg::Matrix modelScale;
	osg::Matrix modelTranslate;
	osg::Matrix modelRot;
	
	osg::MatrixTransform*	unitTransform = new osg::MatrixTransform;
	osg::BoundingSphere bound=modelNode->getBound();

	modelScale.makeScale(sizeModel/bound.radius(),sizeModel/bound.radius(),sizeModel/bound.radius());
    modelRot.makeRotate(0.,osg::Vec3f(1.,0.,0.));
	modelTranslate.makeTranslate(-bound.center());

	unitTransform->postMult(modelTranslate);
	unitTransform->postMult(modelRot);
	unitTransform->postMult(modelScale);

	unitTransform->addChild(modelNode);

	//put model on the marker (not on the middle)
	osg::MatrixTransform*	trackerCenterTransform = new osg::MatrixTransform;

	osg::Matrix artCenter;
	artCenter.makeTranslate(shiftX,shiftY,heightMarker);
	trackerCenterTransform->postMult(artCenter);

	trackerCenterTransform->addChild(unitTransform);

	// create marker
	osg::ref_ptr<osgART::Marker> marker = osgART::TrackerManager::getInstance()->getTracker(0)->getMarker(markerId);
		
	// check before accessing the linked marker
	if (!marker.valid()) {
        
		osg::notify(osg::FATAL) << "No Marker defined!" << std::endl;

		exit(-1);
	}

	// activate the marker
	marker->setActive(true);

	// create a matrix transform related to the marker
	osg::MatrixTransform*  markerTrans = 
		new osgART::ARTTransform(marker.get());

	//put the interaction transform between the node transform and the markerTransform
	interactionTransform->addChild(trackerCenterTransform);

	markerTrans->addChild(interactionTransform);

	//add local light for each marker

	//normalize for environment mapping
	markerTrans->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

	//enable lighting
	markerTrans->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::ON);
	
	//add a light model
	markerTrans->getOrCreateStateSet()->setAttribute(new osg::LightModel());

	//specify some directional ight for the different markers
	markerTrans->addChild(addLight(markerId,markerTrans->getOrCreateStateSet()));

	return markerTrans;
}

void updateInteraction(osg::MatrixTransform& transfo1, osg::MatrixTransform& transfo2)
{
	
	if (osgART::TrackerManager::getInstance()->getTracker(0)->getMarker(0)->isValid()&&
		osgART::TrackerManager::getInstance()->getTracker(0)->getMarker(1)->isValid())
	{
		//get transformation marker 1
		osg::Matrix mat_marker1=osgART::TrackerManager::getInstance()->getTracker(0)->getMarker(0)->getTransform();
		//get transformation marker 2
		osg::Matrix mat_marker2=osgART::TrackerManager::getInstance()->getTracker(0)->getMarker(1)->getTransform();

		//get the relative position of each one
		osg::Vec3f pos1=mat_marker1.getTrans();
		osg::Vec3f pos2=mat_marker2.getTrans();
	
		//compute the euclidian distance between them
		float distance_markers=(pos1-pos2).length();

		std::cerr<<"distance="<<distance_markers<<std::endl;
		//if it's under a threshold
		if (distance_markers<150.)
		{
			//rotate the models upside down
			transfo1.setMatrix(osg::Matrix::rotate(90/57.40,osg::Vec3f(0.,0.,1.)));
			transfo2.setMatrix(osg::Matrix::rotate(0,osg::Vec3f(1.,0.,0.)));
			//PlaySound("sound/gong.wav", NULL, SND_FILENAME | SND_SYNC);
			PlaySound("sound/gong.wav", NULL, SND_FILENAME | SND_ASYNC);
		}
		else
		{
			//otherwise make them to default position
			transfo1.setMatrix(osg::Matrixf::rotate(90/57.4,osg::Vec3f(0.,0.,1.)));
			transfo2.setMatrix(osg::Matrixf::rotate(0,osg::Vec3f(1.,0.,0.)));
		}
	}
	else
	{
			transfo1.setMatrix(osg::Matrixf::rotate(90/57.4,osg::Vec3f(0.,0.,1.)));
			transfo2.setMatrix(osg::Matrixf::rotate(0,osg::Vec3f(1.,0.,0.)));
	}
}


int main(int argc, char* argv[]) {

	if (argc<11)
	{
		std::cout<<"osgartsimple model_name1 size_model shift_x shift_y shift_z model_name2 size_model shift_x shift_y shift_z"<<std::endl;
		exit(-1);
	}
osg::setNotifyLevel(osg::ALWAYS);

	osgARTInit(&argc, argv);
	
	osgProducer::Viewer viewer;
	viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);
	viewer.getCullSettings().setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

#ifndef __linux
	// somehow on Ubuntu Dapper this ends up in a segmentation fault
	viewer.getCamera(0)->getRenderSurface()->fullScreen(false);
#endif

	/* load a video plugin */
	osg::ref_ptr<osgART::GenericVideo> video = osgART::VideoManager::createVideoFromPlugin("osgart_artoolkit");

	/* open the video */
	video->open();

	osg::ref_ptr<osgART::GenericTracker> tracker = 
		osgART::TrackerManager::createTrackerFromPlugin("osgart_artoolkit_tracker");

	
	//add a list of markers with two markers at least
	tracker->init(video->getWidth(), video->getHeight(),"data/markers_list2.dat");

	//Adding video background
	osg::Group* foregroundGroup	= new osg::Group();

	osgART::VideoBackground* videoBackground=new osgART::VideoBackground(video.get());

	videoBackground->setTextureMode(osgART::GenericVideoObject::USE_TEXTURE_RECTANGLE);

	videoBackground->init();

	foregroundGroup->addChild(videoBackground);

	foregroundGroup->getOrCreateStateSet()->setRenderBinDetails(2, "RenderBin");

	osg::Projection* projectionMatrix = new osg::Projection(osg::Matrix(tracker->getProjectionMatrix()));
	
	//create scene to hold all markers
	osg::Group* sceneGroup = new osg::Group();
	sceneGroup->getOrCreateStateSet()->setRenderBinDetails(5, "RenderBin");
	
	//using 2 markers with two different models
	//REM:you need to be sure the markers are defined and exist on the marker file list

	//defined two interaction transformation node we can use later
	osg::MatrixTransform* interactionTransfo=new osg::MatrixTransform;
	
	osg::MatrixTransform* interactionTransfo2=new osg::MatrixTransform;

	//pass the interaction transformation node to the object creation function
	sceneGroup->addChild(addARTModel(interactionTransfo,0,argv[1],atof(argv[2]),atof(argv[3]),atof(argv[4]),atof(argv[5])));
	
	sceneGroup->addChild(addARTModel(interactionTransfo2,1,argv[6],atof(argv[7]),atof(argv[8]),atof(argv[9]),atof(argv[10])));

	foregroundGroup->addChild(sceneGroup);

	osg::MatrixTransform* modelViewMatrix = new osg::MatrixTransform();
	modelViewMatrix->addChild(foregroundGroup);
	projectionMatrix->addChild(modelViewMatrix);
	
	osg::Group* root = new osg::Group();
	root->addChild(projectionMatrix);

	viewer.setSceneData(root);

	viewer.realize();
	
	video->start();
	
    while (!viewer.done()) {
		
		viewer.sync();	
		
		video->update();

		tracker->setImage(video.get());
		tracker->update();
		
		//update the interaction
		updateInteraction(*interactionTransfo,*interactionTransfo2);

        viewer.update();
        viewer.frame();
	
    }
    
	viewer.sync();
    viewer.cleanup_frame();
    viewer.sync();

	video->stop();
	video->close();
	
}
