#include "GLCommon.h"
#include <Windows.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/gtc/matrix_transform.hpp>
// glm::translate, glm::rotate, glm::scale, glm::perspective
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

#include <stdlib.h>		// c libs
#include <stdio.h>		// c libs

#include <iostream>		// C++ IO standard stuff
#include <map>			// Map aka "dictonary" 

#include "cModelLoader.h"			
#include "cVAOManager.h"		// NEW
//#include "cGameObject.h"

#include "cShaderManager.h"

#include <sstream>
#include <fstream>

#include <limits.h>
#include <float.h>

// The Physics function
#include "PhysicsStuff.h"
#include "cPhysics.h"

#include "DebugRenderer/cDebugRenderer.h"
#include <pugixml/pugixml.hpp>
#include <pugixml/pugixml.cpp>
#include "cLight.h"
#include "cMediator.h"
#include "cObjectFactory.h"
#include "AABBStuff.h"

// Used to visualize the attenuation of the lights...
#include "LightManager/cLightHelper.h"

using namespace pugi;

xml_document document;
std::string gameDataLocation = "gameData.xml";
xml_parse_result result = document.load_file(gameDataLocation.c_str());
std::ofstream file;
xml_node root_node = document.child("GameData");
xml_node lightData = root_node.child("LightData");
xml_node rampData = root_node.child("RampData");
xml_node ballData = root_node.child("BallData");
xml_node ballLightData = root_node.child("BallLightData");

void CalcAABBsForMeshModel(cMesh& theMesh);

extern std::map<unsigned long long /*ID*/, cAABB*> g_mapAABBs_World;

cAABB* pCurrentAABB;

bool fileChanged = false;

void DrawObject(glm::mat4 m, iObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager);

//glm::vec3 borderLight3 = glm::vec3(0.0f, -149.0f, 0.0f);
//glm::vec3 borderLight4 = glm::vec3(0.0f, 200.0f, 0.0f);
//glm::vec3 borderLight5 = glm::vec3(0.0f, 0.0f, -199.0f);
//glm::vec3 borderLight6 = glm::vec3(0.0f, 0.0f, 199.0f);

cMediator* pMediator = cMediator::createMediator();

unsigned int currentRamp = 0;

cLight* pMainLight = new cLight();
cLight* pMainLight1 = new cLight();
std::vector<cLight*> pLightsVec;
unsigned int currentLight = 0;
cLight* pCorner1Light = new cLight();
cLight* pCorner2Light = new cLight();
cLight* pCorner3Light = new cLight();
cLight* pCorner4Light = new cLight();

float cameraLeftRight = 0.0f;

glm::vec3 cameraEye = glm::vec3(0.0, 80.0, -280.0);
glm::vec3 cameraTarget = glm::vec3(pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ());
glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);

float SpotInnerAngle1 = 5.0f;
float cornerLightSpotOuterAngle1 = 7.5f;

//mainLight
// This is a "normalized" direction
// (i.e. the length is 1.0f)

bool bLightDebugSheresOn = false;

bool laserActive = false;

bool onGround = false;
bool onPlatform = false;

glm::mat4 calculateWorldMatrix(iObject* pCurrentObject);


// Load up my "scene"  (now global)
std::vector<iObject*> g_vec_pGameObjects;
std::vector<iObject*> g_vec_pEnvironmentObjects;
std::map<std::string /*FriendlyName*/, iObject*> g_map_GameObjectsByFriendlyName;


// returns NULL (0) if we didn't find it.
iObject* pFindObjectByFriendlyName(std::string name);
iObject* pFindObjectByFriendlyNameMap(std::string name);

//bool g_BallCollided = false;

bool isShiftKeyDownByAlone(int mods)
{
	if (mods == GLFW_MOD_SHIFT)
	{
		// Shift key is down all by itself
		return true;
	}
	return false;
}

bool isCtrlKeyDownByAlone(int mods)
{
	if (mods == GLFW_MOD_CONTROL)
	{
		return true;
	}
	return false;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{

	const float CAMERASPEED = 1.0f;
	const float MOVESPEED = 5.0f;

	if (!isShiftKeyDownByAlone(mods) && !isCtrlKeyDownByAlone(mods))
	{

		// Move the camera (A & D for left and right, along the x axis)
		if (key == GLFW_KEY_A)
		{
			cameraEye.x -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_D)
		{
			cameraEye.x += CAMERASPEED;		// Move the camera +0.01f units
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			cameraEye.y -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_E)
		{
			cameraEye.y += CAMERASPEED;		// Move the camera +0.01f units
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_W)
		{
			cameraEye.z -= CAMERASPEED;		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_S)
		{
			cameraEye.z += CAMERASPEED;		// Move the camera +0.01f units
		}

		if (key == GLFW_KEY_B)
		{
//			// Shoot a bullet from the pirate ship
//			// Find the pirate ship...
//			// returns NULL (0) if we didn't find it.
////			cGameObject* pShip = pFindObjectByFriendlyName("PirateShip");
//			iObject* pShip = pFindObjectByFriendlyNameMap("PirateShip");
//			// Maybe check to see if it returned something... 
//
//			// Find the sphere#2
////			cGameObject* pBall = pFindObjectByFriendlyName("Sphere#2");
//			iObject* pBall = pFindObjectByFriendlyNameMap("Sphere#2");
//
//			// Set the location velocity for sphere#2
//			pBall->positionXYZ = pShip->positionXYZ;
//			pBall->inverseMass = 1.0f;		// So it's updated
//			// 20.0 units "to the right"
//			// 30.0 units "up"
//			pBall->velocity = glm::vec3(15.0f, 20.0f, 0.0f);
//			pBall->accel = glm::vec3(0.0f, 0.0f, 0.0f);
//			pBall->diffuseColour = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		}//if ( key == GLFW_KEY_B )

	}

	if (isShiftKeyDownByAlone(mods))
	{
		if (key == GLFW_KEY_9)
		{
			bLightDebugSheresOn = false;
		}
		if (key == GLFW_KEY_0)
		{
			bLightDebugSheresOn = true;
		}
		// switch lights to control
		if (key == GLFW_KEY_M)
		{
			currentLight = 0;		// Move the camera -0.01f units
		}
		// move the light
		if (key == GLFW_KEY_A)
		{
			pLightsVec.at(0)->_PositionX -= CAMERASPEED;
		}
		if (key == GLFW_KEY_D)
		{
			pLightsVec.at(0)->_PositionX += CAMERASPEED;
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			pLightsVec.at(0)->_PositionY -= CAMERASPEED;
		}
		if (key == GLFW_KEY_E)
		{
			pLightsVec.at(0)->_PositionY += CAMERASPEED;
		}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_W)
		{
			pLightsVec.at(0)->_PositionZ -= CAMERASPEED;
		}
		if (key == GLFW_KEY_S)
		{
			pLightsVec.at(0)->_PositionZ += CAMERASPEED;
		}

		if (key == GLFW_KEY_K)
		{
			for (int i = 0; i < pLightsVec.size(); i++)
			{
				std::string currentNodeName = pLightsVec.at(i)->getNodeName();
				xml_node LightToChange = lightData.child(currentNodeName.c_str());
				std::vector<std::string> changeData = pLightsVec.at(i)->getAllDataStrings();
				//int index = 0;
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionX()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionY()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getPositionZ()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getConstAtten()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getLinearAtten()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getQuadraticAtten()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getInnerSpot()));
				//changeData.push_back(std::to_string(pLightsVec.at(i)->getOuterSpot()));
				//Set data to xml to set positions

				int index = 0;
				for (xml_node dataNode = LightToChange.child("PositionX"); dataNode; dataNode = dataNode.next_sibling())
				{
					//LightToChange->first_node("PositionX")->value(changeData.at(i).c_str());
					//LightToChange->first_node("PositionY")->value(changeData.at(i).c_str());
					//LightToChange->first_node("PositionZ")->value(changeData.at(i).c_str());
					//LightToChange->first_node("ConstAtten")->value(changeData.at(i).c_str());
					//LightToChange->first_node("LinearAtten")->value(changeData.at(i).c_str());
					//LightToChange->first_node("QuadraticAtten")->value(changeData.at(i).c_str());
					//LightToChange->first_node("SpotInnerAngle")->value(changeData.at(i).c_str());
					//LightToChange->first_node("SpotOuterAngle")->value(changeData.at(i).c_str());

					//std::string changeString = changeData.at(index);
					//std::cout << changeString << std::endl;
					dataNode.last_child().set_value(changeData.at(index).c_str());
					//std::cout << dataNode->value() << std::endl;
					//dataNode = dataNode->next_sibling();
					index++;
				}
				//for (xml_node<>* dataNode = LightToChange->first_node(); dataNode; dataNode = dataNode->next_sibling())
				//{
				//	//assert(index < changeData.size());
				//	const char * stringToChange = changeData.at(index).c_str();
				//	dataNode->value(stringToChange);
				//	file.open(gameDataLocation);
				//	file << "<?xml version='1.0' encoding='utf-8'?>\n";
				//	file << document;
				//	file.close();
				//	index++;
				//}
			}
			fileChanged = true;
		}
		//if (key == GLFW_KEY_V)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_B)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}
		//if (key == GLFW_KEY_N)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition -= 0.1f;
		//}
		//if (key == GLFW_KEY_M)
		//{
		//	if (pCurrentLight == "mainLight")
		//		mainLightPosition += 0.1f;
		//}


		if (key == GLFW_KEY_9)
		{
			bLightDebugSheresOn = false;
		}

	}//if (isShiftKeyDownByAlone(mods))

	if (isCtrlKeyDownByAlone(mods))
	{
		// move the shpere
		iObject* pSphere = pFindObjectByFriendlyName("Sphere#1");
		if (key == GLFW_KEY_D)
		{
			//pSphere->rotationXYZ -= glm::vec3(CAMERASPEED, 0.0f, 0.0f);
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(MOVESPEED, 0.0f, 0.0f));		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_A)
		{
			//pSphere->rotationXYZ += glm::vec3(CAMERASPEED, 0.0f, 0.0f);
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(MOVESPEED, 0.0f, 0.0f));		// Move the camera +0.01f units
		}

		if (key == GLFW_KEY_M && action == GLFW_PRESS)
		{
			if (pSphere->getIsWireframe())
			{
				pSphere->setIsWireframe(false);
			}
			else
			{
				pSphere->setIsWireframe(true);
			}
		}

		// Move the camera (Q & E for up and down, along the y axis)
		if (key == GLFW_KEY_Q)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(0.0f, MOVESPEED, 0.0f));			// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_E)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(0.0f, MOVESPEED, 0.0f));			// Move the camera +0.01f units
			
		}
		//if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
		//{
		//	if (onGround)
		//	{
		//		pSphere->velocity.y = 10.0f;
		//	}
		//}

		// Move the camera (W & S for towards and away, along the z axis)
		if (key == GLFW_KEY_S)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(0.0f, 0.0f, MOVESPEED));		// Move the camera -0.01f units
		}
		if (key == GLFW_KEY_W)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(0.0f, 0.0f, MOVESPEED));		// Move the camera +0.01f units
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(MOVESPEED, 0.0f, 0.0f));
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(0.0f, 0.0f, MOVESPEED));
		}
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(MOVESPEED, 0.0f, 0.0f));
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(0.0f, 0.0f, MOVESPEED));
		}
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + glm::vec3(MOVESPEED, 0.0f, 0.0f));
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(0.0f, 0.0f, MOVESPEED));
		}
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		{
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(0.0f, 0.0f, MOVESPEED));
			pSphere->setPositionXYZ(pSphere->getPositionXYZ() - glm::vec3(MOVESPEED, 0.0f, 0.0f));
		}

		if (key == GLFW_KEY_1)
		{
			pLightsVec.at(currentLight)->setConstAtten(pLightsVec.at(currentLight)->getConstAtten() * 0.99f);			// 99% of what it was
		}
		if (key == GLFW_KEY_2)
		{
			pLightsVec.at(currentLight)->setConstAtten(pLightsVec.at(currentLight)->getConstAtten() * 1.01f);
		}
		if (key == GLFW_KEY_3)
		{
			pLightsVec.at(currentLight)->setLinearAtten(pLightsVec.at(currentLight)->getLinearAtten() * 0.99f);			// 99% of what it was
		}
		if (key == GLFW_KEY_4)
		{
			pLightsVec.at(currentLight)->setLinearAtten(pLightsVec.at(currentLight)->getLinearAtten() * 1.01f);			// 1% more of what it was
		}
		if (key == GLFW_KEY_5)
		{
			pLightsVec.at(currentLight)->setQuadraticAtten(pLightsVec.at(currentLight)->getQuadraticAtten() * 0.99f);
		}
		if (key == GLFW_KEY_6)
		{
			pLightsVec.at(currentLight)->setQuadraticAtten(pLightsVec.at(currentLight)->getQuadraticAtten() * 1.01f);
		}

		//cGameObject* pShip = pFindObjectByFriendlyName("PirateShip");
		//// Turn the ship around
		//if (key == GLFW_KEY_A)
		//{	// Left
		//	pShip->HACK_AngleAroundYAxis -= 0.01f;
		//	pShip->rotationXYZ.y = pShip->HACK_AngleAroundYAxis;
		//}
		//if (key == GLFW_KEY_D)
		//{	// Right
		//	pShip->HACK_AngleAroundYAxis += 0.01f;
		//	pShip->rotationXYZ.y = pShip->HACK_AngleAroundYAxis;
		//}
		//if (key == GLFW_KEY_W)
		//{	// Faster
		//	pShip->HACK_speed += 0.1f;
		//}
		//if (key == GLFW_KEY_S)
		//{	// Slower
		//	pShip->HACK_speed -= 0.1f;
		//}
	}

	if (isCtrlKeyDownByAlone(mods) && isShiftKeyDownByAlone(mods))
	{

	}


	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

}

static void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

template <class T>
T randInRange(T min, T max)
{
	double value =
		min + static_cast <double> (rand())
		/ (static_cast <double> (RAND_MAX / (static_cast<double>(max - min))));
	return static_cast<T>(value);
};

void explode(iObject* sphere)
{
	if (sphere->getScale() > 5.0f)
	{
		sphere->setIsVisible(false);
	}
	else
	{
		sphere->setScale(sphere->getScale() + 0.01f);
	}
}

int main(void)
{
	//std::ifstream gameData(gameDataLocation);


	//std::vector<char> buffer((std::istreambuf_iterator<char>(gameData)), std::istreambuf_iterator<char>());
	//buffer.push_back('\0');

	//document.parse<0>(&buffer[0]);
	//root_node = document.first_node("GameData");
	//lightData = root_node->first_node("LightData");

	//root_node = document.child("GameData");
	//lightData = root_node.child("LightData");

	cModelLoader* pTheModelLoader = new cModelLoader();	// Heap

	cObjectFactory* pFactory = new cObjectFactory();

	GLFWwindow* window;

	glfwSetErrorCallback(error_callback);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	glfwSetKeyCallback(window, key_callback);
	glfwMakeContextCurrent(window);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glfwSwapInterval(1);



	cDebugRenderer* pDebugRenderer = new cDebugRenderer();
	pDebugRenderer->initialize();

	//cMesh safePartsMesh;
	//pTheModelLoader->LoadPlyModel("assets/models/Parts_Safe_from_Asteroids_xyz_n.ply", safePartsMesh);

	//cMesh exposedPartsMesh;
	//pTheModelLoader->LoadPlyModel("assets/models/Parts_Exposed_to_Asteroids_xyz_n.ply", exposedPartsMesh);

	cMesh mountainRangeMesh;
	pTheModelLoader->LoadPlyModel("assets/models/Mountain_range_xyz_n.ply", mountainRangeMesh);

	cMesh cubeMesh;
	pTheModelLoader->LoadPlyModel("assets/models/Cube_1_Unit_from_origin_XYZ_n.ply", cubeMesh);

	cMesh sphereMesh;
	pTheModelLoader->LoadPlyModel("assets/models/Cube_1_Unit_from_origin_XYZ_n.ply", sphereMesh);

	cMesh asteroid011Mesh;
	pTheModelLoader->LoadPlyModel("assets/models/Asteroid_011_xyz_n.ply", asteroid011Mesh);

	cMesh asteroid014Mesh;
	pTheModelLoader->LoadPlyModel("assets/models/Asteroid_014_xyz_n.ply", asteroid014Mesh);

	cMesh asteroid015Mesh;
	pTheModelLoader->LoadPlyModel("assets/models/Asteroid_015_xyz_n.ply", asteroid015Mesh);

	CalcAABBsForMeshModel(mountainRangeMesh);

	cShaderManager* pTheShaderManager = new cShaderManager();

	cShaderManager::cShader vertexShad;
	vertexShad.fileName = "assets/shaders/vertexShader01.glsl";

	cShaderManager::cShader fragShader;
	fragShader.fileName = "assets/shaders/fragmentShader01.glsl";

	if (!pTheShaderManager->createProgramFromFile("SimpleShader", vertexShad, fragShader))
	{
		std::cout << "Error: didn't compile the shader" << std::endl;
		std::cout << pTheShaderManager->getLastError();
		return -1;
	}

	GLuint shaderProgID = pTheShaderManager->getIDFromFriendlyName("SimpleShader");


	// Create a VAO Manager...
	// #include "cVAOManager.h"  (at the top of your file)
	cVAOManager* pTheVAOManager = new cVAOManager();
	sModelDrawInfo sphereMeshInfo;
	pTheVAOManager->LoadModelIntoVAO("sphere", 
									 sphereMesh,		// Sphere mesh info
									 sphereMeshInfo,
									 shaderProgID);

	//sModelDrawInfo safePartsMeshInfo;
	//pTheVAOManager->LoadModelIntoVAO("safeParts",
	//	safePartsMesh,		// Sphere mesh info
	//	sphereMeshInfo,
	//	shaderProgID);

	//sModelDrawInfo exposedPartsMeshInfo;
	//pTheVAOManager->LoadModelIntoVAO("exposedParts",
	//	exposedPartsMesh,		// Sphere mesh info
	//	sphereMeshInfo,
	//	shaderProgID);

	sModelDrawInfo mountainRangeMeshInfo;
	pTheVAOManager->LoadModelIntoVAO("mountainRange", mountainRangeMesh, mountainRangeMeshInfo, shaderProgID);

	sModelDrawInfo asteroid011Info;
	pTheVAOManager->LoadModelIntoVAO("asteroid011",
		asteroid011Mesh,		// Sphere mesh info
		sphereMeshInfo,
		shaderProgID);

	sModelDrawInfo asteroid014Info;
	pTheVAOManager->LoadModelIntoVAO("asteroid014",
		asteroid014Mesh,		// Sphere mesh info
		sphereMeshInfo,
		shaderProgID);

	sModelDrawInfo asteroid015Info;
	pTheVAOManager->LoadModelIntoVAO("asteroid015",
		asteroid015Mesh,		// Sphere mesh info
		sphereMeshInfo,
		shaderProgID);



	// Sphere and cube
	iObject* pSphere = pFactory->CreateObject("sphere");
	pSphere->setMeshName("sphere");
	pSphere->setFriendlyName("Sphere#1");	// We use to search 
	pSphere->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->setScale(1.0f);
	pSphere->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	//pSphere->setDebugColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pSphere->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->setAccel(glm::vec3(0.0f, 0.0f, 0.0f));
	pSphere->set_SPHERE_radius(1.0f);
	pSphere->setInverseMass(0.0f);
	pSphere->setIsVisible(true);
	pSphere->setIsWireframe(false);
	::g_vec_pGameObjects.push_back(pSphere);

	iObject* pMountainRange = pFactory->CreateObject("mesh");
	pMountainRange->setMeshName("mountainRange");
	pMountainRange->setFriendlyName("mountainRange");	// We use to search 
	pMountainRange->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pMountainRange->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pMountainRange->setScale(1.0f);
	pMountainRange->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	//pMountainRange->setDebugColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pMountainRange->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	pMountainRange->setAccel(glm::vec3(0.0f, 0.0f, 0.0f));
	pMountainRange->setInverseMass(0.0f);
	pMountainRange->setIsVisible(true);
	pMountainRange->setIsWireframe(false);
	::g_vec_pEnvironmentObjects.push_back(pMountainRange);

	iObject* pAsteroid011 = pFactory->CreateObject("sphere");
	pAsteroid011->setMeshName("asteroid011");
	pAsteroid011->setFriendlyName("asteroid011");	// We use to search 
	pAsteroid011->setPositionXYZ(glm::vec3(0.0f, 50.0f, 0.0f));
	pAsteroid011->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid011->setScale(0.02f);
	pAsteroid011->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pAsteroid011->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid011->setAccel(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid011->setInverseMass(1.0f);
	pAsteroid011->set_SPHERE_radius(1.0f);
	pAsteroid011->setIsVisible(true);
	pAsteroid011->setIsWireframe(false);
	//::g_vec_pAsteroidObjects.push_back(pAsteroid011);

	iObject* pAsteroid014 = pFactory->CreateObject("sphere");
	pAsteroid014->setMeshName("asteroid014");
	pAsteroid014->setFriendlyName("asteroid014");	// We use to search 
	pAsteroid014->setPositionXYZ(glm::vec3(20.0f, 50.0f, 0.0f));
	pAsteroid014->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid014->setScale(0.02f);
	pAsteroid014->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pAsteroid014->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid014->setAccel(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid014->setInverseMass(1.0f);
	pAsteroid011->set_SPHERE_radius(1.0f);
	pAsteroid014->setIsVisible(true);
	pAsteroid014->setIsWireframe(false);
	//::g_vec_pAsteroidObjects.push_back(pAsteroid014);

	iObject* pAsteroid015 = pFactory->CreateObject("sphere");
	pAsteroid015->setMeshName("asteroid015");
	pAsteroid015->setFriendlyName("asteroid015");	// We use to search 
	pAsteroid015->setPositionXYZ(glm::vec3(-50.0f, 50.0f, 0.0f));
	pAsteroid015->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid015->setScale(0.02f);
	pAsteroid015->setObjectColourRGBA(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
	pAsteroid015->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid015->setAccel(glm::vec3(0.0f, 0.0f, 0.0f));
	pAsteroid015->setInverseMass(1.0f);
	pAsteroid011->set_SPHERE_radius(1.0f);
	pAsteroid015->setIsVisible(true);
	pAsteroid015->setIsWireframe(false);
	//::g_vec_pAsteroidObjects.push_back(pAsteroid015);
	
	// Will be moved placed around the scene
	iObject* pDebugSphere = pFactory->CreateObject("sphere");
	pDebugSphere->setMeshName("sphere");
	pDebugSphere->setFriendlyName("debug_sphere");
	pDebugSphere->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pDebugSphere->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pDebugSphere->setScale(0.1f);
	//	pDebugSphere->objectColourRGBA = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
	pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
	pDebugSphere->setIsWireframe(true);
	pDebugSphere->setInverseMass(0.0f);			// Sphere won't move
	pDebugSphere->setIsVisible(false);

	iObject* pDebugCube = pFactory->CreateObject("mesh");
	pDebugCube->setMeshName("sphere");
	pDebugCube->setFriendlyName("debug_cube");
	pDebugCube->setPositionXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pDebugCube->setRotationXYZ(glm::vec3(0.0f, 0.0f, 0.0f));
	pDebugCube->setScale(10.0f);
	pDebugCube->setDebugColour(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
	pDebugCube->setIsWireframe(true);
	pDebugCube->setInverseMass(0.0f);			// Sphere won't move
	pDebugCube->setIsVisible(false);

	glEnable(GL_DEPTH);			// Write to the depth buffer
	glEnable(GL_DEPTH_TEST);	// Test with buffer when drawing

	cPhysics* pPhsyics = new cPhysics();

	cLightHelper* pLightHelper = new cLightHelper();

	//Get data from xml to set positions of main light
	pMainLight->setNodeName("MainLight");
	xml_node mainLightNode = lightData.child("MainLight");
	pMainLight->setPositionX(std::stof(mainLightNode.child("PositionX").child_value()));
	pMainLight->setPositionY(std::stof(mainLightNode.child("PositionY").child_value()));
	pMainLight->setPositionZ(std::stof(mainLightNode.child("PositionZ").child_value()));
	pMainLight->setPositionXYZ(glm::vec3(std::stof(mainLightNode.child("PositionX").child_value()), std::stof(mainLightNode.child("PositionY").child_value()), std::stof(mainLightNode.child("PositionZ").child_value())));
	pMainLight->setConstAtten(std::stof(mainLightNode.child("ConstAtten").child_value()));
	pMainLight->setLinearAtten(std::stof(mainLightNode.child("LinearAtten").child_value()));
	pMainLight->setQuadraticAtten(std::stof(mainLightNode.child("QuadraticAtten").child_value()));
	pMainLight->setInnerSpot(std::stof(mainLightNode.child("SpotInnerAngle").child_value()));
	pMainLight->setOuterSpot(std::stof(mainLightNode.child("SpotOuterAngle").child_value()));

	pMainLight1->setNodeName("MainLight1");
	xml_node mainLight1Node = lightData.child("MainLight1");
	pMainLight1->setPositionX(std::stof(mainLight1Node.child("PositionX").child_value()));
	pMainLight1->setPositionY(std::stof(mainLight1Node.child("PositionY").child_value()));
	pMainLight1->setPositionZ(std::stof(mainLight1Node.child("PositionZ").child_value()));
	pMainLight1->setPositionXYZ(glm::vec3(std::stof(mainLight1Node.child("PositionX").child_value()), std::stof(mainLight1Node.child("PositionY").child_value()), std::stof(mainLight1Node.child("PositionZ").child_value())));
	pMainLight1->setConstAtten(std::stof(mainLight1Node.child("ConstAtten").child_value()));
	pMainLight1->setLinearAtten(std::stof(mainLight1Node.child("LinearAtten").child_value()));
	pMainLight1->setQuadraticAtten(std::stof(mainLight1Node.child("QuadraticAtten").child_value()));
	pMainLight1->setInnerSpot(std::stof(mainLight1Node.child("SpotInnerAngle").child_value()));
	pMainLight1->setOuterSpot(std::stof(mainLight1Node.child("SpotOuterAngle").child_value()));

	pLightsVec.push_back(pMainLight);
	pLightsVec.push_back(pMainLight1);

	int setCount = 0;
	float properYPosition = 0.0f;
	float properPlatformYPosition = 0.0f;

	int time = 1;
	int laserTime = 1;

	bool wasHit = false;
	glm::vec3 hitPosition = glm::vec3(0.0f, 0.0f, 0.0f);

	glm::mat4 matWorld = calculateWorldMatrix(pMountainRange);

	cMesh transformedMesh;
	pPhsyics->CalculateTransformedMesh(mountainRangeMesh, matWorld, transformedMesh);

	cMesh* mountainPtr = &mountainRangeMesh;

	//for (std::map<unsigned long long, cAABB*>::iterator mapIt = g_mapAABBs_World.begin(); mapIt != g_mapAABBs_World.end(); mapIt++)
	//{
	//	if (mapIt->second->vecTriangles.size() == 0)
	//	{
	//		g_mapAABBs_World.erase(mapIt->first);
	//	}
	//}

	while (!glfwWindowShouldClose(window))
	{
		float ratio;
		int width, height;
		glm::mat4 p, v;

		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;

		// Projection matrix
		p = glm::perspective(0.6f,		// FOV
			ratio,			// Aspect ratio
			0.1f,			// Near clipping plane
			1000.0f);		// Far clipping plane

// View matrix
		v = glm::mat4(1.0f);

		glm::vec3 mainLightPosition = glm::vec3(pMainLight->getPositionX(), pMainLight->getPositionY(), pMainLight->getPositionZ());

		
		v = glm::lookAt(cameraEye,
			pSphere->getPositionXYZ(),
			upVector);

		glViewport(0, 0, width, height);

		// Clear both the colour buffer (what we see) and the 
		//  depth (or z) buffer.
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		int lightIndex = 0;
		for (lightIndex; lightIndex < pLightsVec.size(); ++lightIndex)
		{
			std::string positionString = "theLights[" + std::to_string(lightIndex) + "].position";
			std::string diffuseString = "theLights[" + std::to_string(lightIndex) + "].diffuse";
			std::string specularString = "theLights[" + std::to_string(lightIndex) + "].specular";
			std::string attenString = "theLights[" + std::to_string(lightIndex) + "].atten";
			std::string directionString = "theLights[" + std::to_string(lightIndex) + "].direction";
			std::string param1String = "theLights[" + std::to_string(lightIndex) + "].param1";
			std::string param2String = "theLights[" + std::to_string(lightIndex) + "].param2";

			GLint position = glGetUniformLocation(shaderProgID, positionString.c_str());
			GLint diffuse = glGetUniformLocation(shaderProgID, diffuseString.c_str());
			GLint specular = glGetUniformLocation(shaderProgID, specularString.c_str());
			GLint atten = glGetUniformLocation(shaderProgID, attenString.c_str());
			GLint direction = glGetUniformLocation(shaderProgID, directionString.c_str());
			GLint param1 = glGetUniformLocation(shaderProgID, param1String.c_str());
			GLint param2 = glGetUniformLocation(shaderProgID, param2String.c_str());

			glUniform4f(position, pLightsVec.at(lightIndex)->getPositionX(), pLightsVec.at(lightIndex)->getPositionY(), pLightsVec.at(lightIndex)->getPositionZ(), 1.0f);
			glUniform4f(diffuse, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(specular, 1.0f, 1.0f, 1.0f, 1.0f);	// White
			glUniform4f(atten, pLightsVec.at(lightIndex)->getConstAtten(),  /* constant attenuation */	pLightsVec.at(lightIndex)->getLinearAtten(),  /* Linear */ pLightsVec.at(lightIndex)->getQuadraticAtten(),	/* Quadratic */  1000000.0f);	// Distance cut off

			glUniform4f(param1, 0.0f /*POINT light*/, 0.0f, 0.0f, 1.0f);
			glUniform4f(param2, 1.0f /*Light is on*/, 0.0f, 0.0f, 1.0f);
		}

		GLint eyeLocation_UL = glGetUniformLocation(shaderProgID, "eyeLocation");

		glUniform4f(eyeLocation_UL,
			cameraEye.x, cameraEye.y, cameraEye.z, 1.0f);

		
		unsigned long long pID = cAABB::get_ID_of_AABB_I_Might_Be_In(pSphere->getPositionXYZ());
		pCurrentAABB = g_mapAABBs_World.begin()->second;
		if (pID)
		{
			std::stringstream ssTitle;
			ssTitle
				<< "The sphere is at: "
				<< pSphere->getPositionXYZ().x << ", "
				<< pSphere->getPositionXYZ().y << ", "
				<< pSphere->getPositionXYZ().z << ", "
				<< " total AABBs: " << g_mapAABBs_World.size()
				<< "   and is inside AABB: " << pID
				<< " which has " << ::g_mapAABBs_World.find(pID)->second->vecTriangles.size() << std::endl;
			glfwSetWindowTitle(window, ssTitle.str().c_str());
			pCurrentAABB = g_mapAABBs_World.find(pID)->second;
		}


		//pCurrentAABB = g_mapAABBs_World.find(pID)->second;
		int cubeCount = 0;
		int overlappingCubes = 0;
		std::vector<std::string> locationVec;
		bool cubeAtOrigin = false;
		for (std::map<unsigned long long, cAABB*>::iterator mapIt = g_mapAABBs_World.begin(); mapIt != g_mapAABBs_World.end(); mapIt++)
		{
			//std::map<unsigned long long, cAABB*>::iterator mapIt2 = mapIt++;
			{// Draw the AABBs
				glm::mat4 matModel = glm::mat4(1.0f);
				if (mapIt->second)
				{
					pDebugCube->setPositionXYZ(mapIt->second->getCentre());
					if (mapIt->second->minXYZ.x < -999.0f && mapIt->second->minXYZ.y < -199.0f && mapIt->second->minXYZ.z < -999.0f)
					{
						cubeAtOrigin = true;
					}
					std::string positionString = std::to_string(pDebugCube->getPositionXYZ().x) + ", " + std::to_string(pDebugCube->getPositionXYZ().y) + ", " + std::to_string(pDebugCube->getPositionXYZ().z);
					locationVec.push_back(positionString);
					pDebugCube->setScale(50.0f / 2.0f);
					if (mapIt->first == pID)
					{
						pDebugCube->setDebugColour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
					}
					else
					{
						pDebugCube->setDebugColour(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
					}
					if (mapIt->first == pID)
					{
					pDebugCube->setIsWireframe(true);
					DrawObject(matModel, pDebugCube,
						shaderProgID, pTheVAOManager);
					}
				}
			}
			/*if (mapIt->second->minXYZ == mapIt2->second->minXYZ)
			{
				overlappingCubes++;
			}*/
			cubeCount++;
		}


		for (int i = 0; i < locationVec.size(); i++)
		{
			for (int j = 0; j < locationVec.size(); j++)
			{
				if (locationVec.at(i) == locationVec.at(j) && i != j)
				{
					overlappingCubes++;
				}
			}
		}
		std::vector<glm::vec3> pointVec = pCurrentAABB->getVecBoxPoints();

		//{// Draw the AABBs
		//		glm::mat4 matModel = glm::mat4(1.0f);
		//		pDebugCube->setPositionXYZ(pointVec.at(0));
		//		pDebugCube->setScale(10.0f);
		//		pDebugCube->setDebugColour(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
		//		pDebugCube->setIsWireframe(true);
		//		DrawObject(matModel, pDebugCube,
		//			shaderProgID, pTheVAOManager);
		//}



		/*std::cout << "Point 1: " << "X: " << pointVec.at(i).x << " Y: " << pointVec.at(i).y << " Z: " << pointVec.at(i).z << "  -  " << "Point 1: " << "X: " << pointVec.at(i + 1).x << " Y: " << pointVec.at(i + 1).y << " Z: " << pointVec.at(i + 1).z;
		std::string error = pDebugRenderer->getLastError();
		std::cout << error;*/


		GLint matView_UL = glGetUniformLocation(shaderProgID, "matView");
		GLint matProj_UL = glGetUniformLocation(shaderProgID, "matProj");

		glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(v));
		glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(p));


		// **************************************************
		// **************************************************
		// Loop to draw everything in the scene

		for (int index = 0; index != ::g_vec_pGameObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			iObject* pCurrentObject = ::g_vec_pGameObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}//for (int index...
		for (int index = 0; index != ::g_vec_pEnvironmentObjects.size(); index++)
		{
			glm::mat4 matModel = glm::mat4(1.0f);

			iObject* pCurrentObject = ::g_vec_pEnvironmentObjects[index];

			DrawObject(matModel, pCurrentObject,
				shaderProgID, pTheVAOManager);

		}//for (int index...

		//pPhsyics->IntegrationStep(g_vec_pGameObjects, 0.03f);

		for (int k = 0; k < pCurrentAABB->vecTriangles.size(); k++)
		{
			//std::cout << pCurrentAABB->vecTriangles.size() << std::endl;
			glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
			cPhysics::sPhysicsTriangle closestTriangle;

			//cMesh transformedMesh;
			//pPhsyics->CalculateTransformedMesh(mountainRangeMesh, matWorld, transformedMesh);

			pPhsyics->GetClosestTriangleToPoint(pSphere->getPositionXYZ(), &mountainRangeMesh, pCurrentAABB, closestPoint, closestTriangle);

			// Highlight the triangle that I'm closest to
			//pDebugRenderer->addTriangle(closestTriangle.verts[0],
			//	closestTriangle.verts[1],
			//	closestTriangle.verts[2],
			//	glm::vec3(1.0f, 0.0f, 0.0f));

			//// Highlight the triangle that I'm closest to
			//// To draw the normal, calculate the average of the 3 vertices, 
			//// then draw that average + the normal (the normal starts at the 0,0,0 OF THE TRIANGLE)
			//glm::vec3 centreOfTriangle = (closestTriangle.verts[0] +
			//	closestTriangle.verts[1] +
			//	closestTriangle.verts[2]) / 3.0f;		// Average

			//glm::vec3 normalInWorld = centreOfTriangle + (closestTriangle.normal * 20.0f);	// Normal x 10 length

			//pDebugRenderer->addLine(centreOfTriangle,
			//	normalInWorld,
			//	glm::vec3(1.0f, 1.0f, 0.0f));

			// Are we hitting the triangle? 
			float distance = glm::length(pSphere->getPositionXYZ() - closestPoint);

	//		if (distance <= pSphere->get_SPHERE_radius())
	//		{
	//			//if (k == 0)
	//			//{
	//			//	pSphere->inverseMass = 0.0f;
	//			//	pSphere->positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
	//			//}

	//			// ************************************************************************

	//			// If you want, move the sphere back to where it just penetrated...
	//			// So that it will collide exactly where it's supposed to. 
	//			// But, that's not a big problem.

	//			// 1. Calculate vector from centre of sphere to closest point
	//			glm::vec3 vecSphereToClosestPoint = closestPoint - pSphere->getPositionXYZ();

	//			// 2. Get the length of this vector
	//			float centreToContractDistance = glm::length(vecSphereToClosestPoint);

	//			// 3. Create a vector from closest point to radius
	//			float lengthPositionAdjustment = pSphere->get_SPHERE_radius() - centreToContractDistance;

	//			// 4. Sphere is moving in the direction of the velocity, so 
	//			//    we want to move the sphere BACK along this velocity vector
	//			glm::vec3 vecDirection = glm::normalize(pSphere->getVelocity());

	//			glm::vec3 vecPositionAdjust = (-vecDirection) * lengthPositionAdjustment;

	//			// 5. Reposition sphere 
	//			pSphere->setPositionXYZ(pSphere->getPositionXYZ() + vecPositionAdjust);
	//			//			pSphere->inverseMass = 0.0f;

	//						// ************************************************************************


	//						// Is in contact with the triangle... 
	//						// Calculate the response vector off the triangle. 
	//			glm::vec3 velocityVector = glm::normalize(pSphere->getVelocity());
	//			float gravY = (-pSphere->getVelocity().y) * 0.45f;
	//			glm::vec3 gravity = glm::vec3(0.0f, gravY, 0.0f);

	//			// closestTriangle.normal
	//			glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
	//			reflectionVec = glm::normalize(reflectionVec);

	//			// Stop the sphere and draw the two vectors...
	////			pSphere->inverseMass = 0.0f;	// Stopped

	//			glm::vec3 velVecX20 = velocityVector * 10.0f;
	//			pDebugRenderer->addLine(closestPoint, velVecX20,
	//				glm::vec3(1.0f, 0.0f, 0.0f), 30.0f /*seconds*/);

	//			glm::vec3 reflectionVecX20 = reflectionVec * 10.0f;
	//			pDebugRenderer->addLine(closestPoint, reflectionVecX20,
	//				glm::vec3(0.0f, 1.0f, 1.0f), 30.0f /*seconds*/);

	//			// Change the direction of the ball (the bounce off the triangle)

	//			// Get lenght of the velocity vector
	//			float speed = glm::length(pSphere->getVelocity());



	//			pSphere->setDiffuseColour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	//			//std::cout << pSphere->velocity.b << ", " << pSphere->velocity.g << ", " << pSphere->velocity.p << ", " << pSphere->velocity.r << ", " << pSphere->velocity.s << ", " << pSphere->velocity.t << ", " << pSphere->velocity.x << ", " << pSphere->velocity.y << ", " << pSphere->velocity.z;
	//		}

			/*bool DidBallCollideWithGround = false;
			HACK_BounceOffSomePlanes(pSphere, DidBallCollideWithGround );*/

			// A more general 
			//pPhsyics->TestForCollisions(::g_vec_pGameObjects);

			{// Draw closest point
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(closestPoint);
				pDebugSphere->setScale(1.0f);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}


			// How far did we penetrate the surface?
			glm::vec3 CentreToClosestPoint = pSphere->getPositionXYZ() - closestPoint;

			// Direction that ball is going is normalized velocity
			glm::vec3 directionBall = glm::normalize(pSphere->getVelocity());	// 1.0f

			// Calcualte direction to move it back the way it came from
			glm::vec3 oppositeDirection = -directionBall;				// 1.0f

			float distanceToClosestPoint = glm::length(CentreToClosestPoint);

			pDebugRenderer->addLine(pSphere->getPositionXYZ(),
				closestPoint,
				glm::vec3(0.0f, 1.0f, 0.0f),
				1.0f);

		}// end for
		

		if (bLightDebugSheresOn)
		{
			{// Draw where the light is at
				for (int i = 0; i < pLightsVec.size(); ++i)
				{
					glm::mat4 matModel = glm::mat4(1.0f);
					pDebugSphere->setPositionXYZ(pLightsVec.at(i)->getPositionXYZ());
					pDebugSphere->setScale(0.5f);
					pDebugSphere->setDebugColour(glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
					pDebugSphere->setIsWireframe(true);
					DrawObject(matModel, pDebugSphere,
						shaderProgID, pTheVAOManager);
					pDebugSphere->setIsVisible(true);
				}
				/*
				//glm::mat4 matModel1 = glm::mat4(1.0f);
				//pDebugSphere->positionXYZ = corner1LightPosition;
				//pDebugSphere->scale = 0.5f;
				//pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				//pDebugSphere->isWireframe = true;
				//DrawObject(matModel1, pDebugSphere,
				//	shaderProgID, pTheVAOManager);
				//
				glm::mat4 matModel2 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight2;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel2, pDebugSphere,
					shaderProgID, pTheVAOManager);
					//
				glm::mat4 matModel3 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight3;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel3, pDebugSphere,
					shaderProgID, pTheVAOManager);

				glm::mat4 matModel4 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight4;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel4, pDebugSphere,
					shaderProgID, pTheVAOManager);

				glm::mat4 matModel5 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight5;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel5, pDebugSphere,
					shaderProgID, pTheVAOManager);

				glm::mat4 matModel6 = glm::mat4(1.0f);
				pDebugSphere->positionXYZ = borderLight6;
				pDebugSphere->scale = 0.5f;
				pDebugSphere->debugColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				pDebugSphere->isWireframe = true;
				DrawObject(matModel6, pDebugSphere,
					shaderProgID, pTheVAOManager);*/
			}

			// Draw spheres to represent the attenuation...
			{   // Draw a sphere at 1% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pLightsVec.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.01f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pLightsVec.at(currentLight)->getConstAtten(),
					pLightsVec.at(currentLight)->getLinearAtten(),
					pLightsVec.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 25% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pLightsVec.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.25f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pLightsVec.at(currentLight)->getConstAtten(),
					pLightsVec.at(currentLight)->getLinearAtten(),
					pLightsVec.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 50% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pLightsVec.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.50f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pLightsVec.at(currentLight)->getConstAtten(),
					pLightsVec.at(currentLight)->getLinearAtten(),
					pLightsVec.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(0.0f, 1.0f, 1.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 75% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pLightsVec.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.75f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pLightsVec.at(currentLight)->getConstAtten(),
					pLightsVec.at(currentLight)->getLinearAtten(),
					pLightsVec.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(1.0f, 1.0f, 0.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
			{   // Draw a sphere at 95% brightness
				glm::mat4 matModel = glm::mat4(1.0f);
				pDebugSphere->setPositionXYZ(pLightsVec.at(currentLight)->getPositionXYZ());
				float sphereSize = pLightHelper->calcApproxDistFromAtten(
					0.95f,		// 1% brightness (essentially black)
					0.001f,		// Within 0.1%  
					100000.0f,	// Will quit when it's at this distance
					pLightsVec.at(currentLight)->getConstAtten(),
					pLightsVec.at(currentLight)->getLinearAtten(),
					pLightsVec.at(currentLight)->getQuadraticAtten());
				pDebugSphere->setScale(sphereSize);
				pDebugSphere->setDebugColour(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
				pDebugSphere->setIsWireframe(true);
				DrawObject(matModel, pDebugSphere,
					shaderProgID, pTheVAOManager);
			}
		}// if (bLightDebugSheresOn) 

		 // **************************************************
		// *************************************************
		if (fileChanged)
		{
			//file.open(gameDataLocation);
			file << "<?xml version='1.0' encoding='utf-8'?>\n";
			document.save_file(gameDataLocation.c_str());
			//file.close();
		}



		pDebugRenderer->addLine(pointVec.at(0), pointVec.at(1), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(0), pointVec.at(3), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(1), pointVec.at(2), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(2), pointVec.at(3), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);

		pDebugRenderer->addLine(pointVec.at(4), pointVec.at(5), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(5), pointVec.at(6), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(6), pointVec.at(7), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(4), pointVec.at(7), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);

		pDebugRenderer->addLine(pointVec.at(0), pointVec.at(4), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(1), pointVec.at(5), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(2), pointVec.at(6), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);
		pDebugRenderer->addLine(pointVec.at(3), pointVec.at(7), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f);

		//pDebugRenderer->RenderDebugObjects(v, p, 0.03f);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}// main loop

	glfwDestroyWindow(window);
	glfwTerminate();

	// Delete everything
	delete pTheModelLoader;
	//	delete pTheVAOManager;

		// Watch out!!
		// sVertex* pVertices = new sVertex[numberOfVertsOnGPU];
	//	delete [] pVertices;		// If it's an array, also use [] bracket

	exit(EXIT_SUCCESS);
}


void DrawObject(glm::mat4 m, iObject* pCurrentObject, GLint shaderProgID, cVAOManager* pVAOManager)
{
	// 
				//         mat4x4_identity(m);
	m = glm::mat4(1.0f);



	// ******* TRANSLATION TRANSFORM *********
	glm::mat4 matTrans
		= glm::translate(glm::mat4(1.0f),
			glm::vec3(pCurrentObject->getPositionXYZ().x,
				pCurrentObject->getPositionXYZ().y,
				pCurrentObject->getPositionXYZ().z));
	m = m * matTrans;
	// ******* TRANSLATION TRANSFORM *********



	// ******* ROTATION TRANSFORM *********
	//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().z,					// Angle 
		glm::vec3(0.0f, 0.0f, 1.0f));
	m = m * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().y,	//(float)glfwGetTime(),					// Angle 
		glm::vec3(0.0f, 1.0f, 0.0f));
	m = m * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().x,	// (float)glfwGetTime(),					// Angle 
		glm::vec3(1.0f, 0.0f, 0.0f));
	m = m * rotateX;
	// ******* ROTATION TRANSFORM *********



	// ******* SCALE TRANSFORM *********
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->getScale(),
			pCurrentObject->getScale(),
			pCurrentObject->getScale()));
	m = m * scale;
	// ******* SCALE TRANSFORM *********



	//mat4x4_mul(mvp, p, m);
	//mvp = p * v * m;

	// Choose which shader to use
	//glUseProgram(program);
	glUseProgram(shaderProgID);


	//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
	//glUniformMatrix4fv(mvp_location, 1, GL_FALSE, glm::value_ptr(mvp));

	//uniform mat4 matModel;		// Model or World 
	//uniform mat4 matView; 		// View or camera
	//uniform mat4 matProj;
	GLint matModel_UL = glGetUniformLocation(shaderProgID, "matModel");

	glUniformMatrix4fv(matModel_UL, 1, GL_FALSE, glm::value_ptr(m));
	//glUniformMatrix4fv(matView_UL, 1, GL_FALSE, glm::value_ptr(v));
	//glUniformMatrix4fv(matProj_UL, 1, GL_FALSE, glm::value_ptr(p));



	// Find the location of the uniform variable newColour
	GLint newColour_location = glGetUniformLocation(shaderProgID, "newColour");

	glUniform3f(newColour_location,
		pCurrentObject->getObjectColourRGBA().r,
		pCurrentObject->getObjectColourRGBA().g,
		pCurrentObject->getObjectColourRGBA().b);

	GLint diffuseColour_UL = glGetUniformLocation(shaderProgID, "diffuseColour");
	glUniform4f(diffuseColour_UL,
		pCurrentObject->getObjectColourRGBA().r,
		pCurrentObject->getObjectColourRGBA().g,
		pCurrentObject->getObjectColourRGBA().b,
		pCurrentObject->getObjectColourRGBA().a);	// 

	GLint specularColour_UL = glGetUniformLocation(shaderProgID, "specularColour");
	glUniform4f(specularColour_UL,
		1.0f,	// R
		1.0f,	// G
		1.0f,	// B
		1000.0f);	// Specular "power" (how shinny the object is)
					// 1.0 to really big (10000.0f)


//uniform vec4 debugColour;
//uniform bool bDoNotLight;
	GLint debugColour_UL = glGetUniformLocation(shaderProgID, "debugColour");
	GLint bDoNotLight_UL = glGetUniformLocation(shaderProgID, "bDoNotLight");

	if (pCurrentObject->getIsWireframe())
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);		// LINES
		glUniform4f(debugColour_UL,
			pCurrentObject->getDebugColour().r,
			pCurrentObject->getDebugColour().g,
			pCurrentObject->getDebugColour().b,
			pCurrentObject->getDebugColour().a);
		glUniform1f(bDoNotLight_UL, (float)GL_TRUE);
	}
	else
	{	// Regular object (lit and not wireframe)
		glUniform1f(bDoNotLight_UL, (float)GL_FALSE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);		// SOLID
	}
	//glPointSize(15.0f);

	if (pCurrentObject->getDisableDepthBufferTest())
	{
		glDisable(GL_DEPTH_TEST);					// DEPTH Test OFF
	}
	else
	{
		glEnable(GL_DEPTH_TEST);						// Turn ON depth test
	}

	if (pCurrentObject->getDisableDepthBufferWrite())
	{
		glDisable(GL_DEPTH);						// DON'T Write to depth buffer
	}
	else
	{
		glEnable(GL_DEPTH);								// Write to depth buffer
	}


	//		glDrawArrays(GL_TRIANGLES, 0, 2844);
	//		glDrawArrays(GL_TRIANGLES, 0, numberOfVertsOnGPU);

	sModelDrawInfo drawInfo;
	//if (pTheVAOManager->FindDrawInfoByModelName("bunny", drawInfo))
	if (pVAOManager->FindDrawInfoByModelName(pCurrentObject->getMeshName(), drawInfo))
	{
		glBindVertexArray(drawInfo.VAO_ID);
		glDrawElements(GL_TRIANGLES,
			drawInfo.numberOfIndices,
			GL_UNSIGNED_INT,
			0);
		glBindVertexArray(0);
	}

	return;
} // DrawObject;
// 

// returns NULL (0) if we didn't find it.
iObject* pFindObjectByFriendlyName(std::string name)
{
	// Do a linear search 
	for (unsigned int index = 0;
		index != g_vec_pGameObjects.size(); index++)
	{
		if (::g_vec_pGameObjects[index]->getFriendlyName() == name)
		{
			// Found it!!
			return ::g_vec_pGameObjects[index];
		}
	}
	// Didn't find it
	return NULL;
}

// returns NULL (0) if we didn't find it.
iObject* pFindObjectByFriendlyNameMap(std::string name)
{
	//std::map<std::string, cGameObject*> g_map_GameObjectsByFriendlyName;
	return ::g_map_GameObjectsByFriendlyName[name];
}

glm::mat4 calculateWorldMatrix(iObject* pCurrentObject)
{

	glm::mat4 matWorld = glm::mat4(1.0f);


	// ******* TRANSLATION TRANSFORM *********
	glm::mat4 matTrans
		= glm::translate(glm::mat4(1.0f),
			glm::vec3(pCurrentObject->getPositionXYZ().x,
				pCurrentObject->getPositionXYZ().y,
				pCurrentObject->getPositionXYZ().z));
	matWorld = matWorld * matTrans;
	// ******* TRANSLATION TRANSFORM *********



	// ******* ROTATION TRANSFORM *********
	//mat4x4_rotate_Z(m, m, (float) glfwGetTime());
	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().z,					// Angle 
		glm::vec3(0.0f, 0.0f, 1.0f));
	matWorld = matWorld * rotateZ;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().y,	//(float)glfwGetTime(),					// Angle 
		glm::vec3(0.0f, 1.0f, 0.0f));
	matWorld = matWorld * rotateY;

	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->getRotationXYZ().x,	// (float)glfwGetTime(),					// Angle 
		glm::vec3(1.0f, 0.0f, 0.0f));
	matWorld = matWorld * rotateX;
	// ******* ROTATION TRANSFORM *********



	// ******* SCALE TRANSFORM *********
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->getScale(),
			pCurrentObject->getScale(),
			pCurrentObject->getScale()));
	matWorld = matWorld * scale;
	// ******* SCALE TRANSFORM *********


	return matWorld;
}