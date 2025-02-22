/* Copyright (c) <2003-2018> <Newton Game Dynamics>
*
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
*
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/

#include "stdafx.h"
#include "ExportFbx.h"
#include "exportMeshNode.h"

#ifdef IOS_REF
	#undef  IOS_REF
	#define IOS_REF (*(manager->GetIOSettings()))
#endif

static int InitializeSdkObjects(FbxManager*& manager, FbxScene*& pScene);
static FbxNode* CreateSkeleton(const exportMeshNode* const model, FbxScene* pScene);
static bool CreateScene(const exportMeshNode* const model, FbxManager *fbxManager, FbxScene* pScene);
static bool SaveScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename, int pFileFormat = -1, bool pEmbedMedia = false);

bool ExportFbx(const exportMeshNode* const scene, const char* const name)
{
	FbxScene* fbxScene = nullptr;
	FbxManager* fbxManager = nullptr;
	if (!InitializeSdkObjects(fbxManager, fbxScene))
	{
		FBXSDK_printf("failed to initialize fbx sdk: %s\n", name);
		return 0;
	}

	//if (ConvertToFbx(bvhSkeleton, fbxScene, exportSkeleton, exportAnimations))
	//{
	////	if (exportAnimations) {
	////		char name[1024];
	////		strcpy(name, argv[1]);
	////		_strlwr(name);
	////		char* ptr = strstr(name, ".fbx");
	////		ptr[0] = 0;
	////		strcat(name, ".anm");
	////		ExportAnimation(name, *bvhScene);
	////	}
	////	else {
	////		char name[1024];
	////		strcpy(name, argv[1]);
	////		_strlwr(name);
	////		char* ptr = strstr(name, ".fbx");
	////		ptr[0] = 0;
	////		strcat(name, ".ngd");
	////		bvhScene->Serialize(name);
	////	}
	//}

	// Create the scene.
	bool lResult = CreateScene(scene, fbxManager, fbxScene);

	if (lResult == false)
	{
		FBXSDK_printf("\n\nAn error occurred while creating the scene...\n");
		fbxManager->Destroy();
		return 0;
	}

	// Save the scene.
	lResult = SaveScene(fbxManager, fbxScene, name);
	if (lResult == false)
	{
		FBXSDK_printf("\n\nAn error occurred while saving the scene...\n");
		fbxManager->Destroy();
		return 0;
	}

	fbxManager->Destroy();
	return true;
}

int InitializeSdkObjects(FbxManager*& manager, FbxScene*& pScene)
{
	//The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
	manager = FbxManager::Create();
	if (!manager)
	{
		FBXSDK_printf("Error: Unable to create FBX Manager!\n");
		exit(1);
	}
	else FBXSDK_printf("Autodesk FBX SDK version %s\n", manager->GetVersion());

	//Create an IOSettings object. This object holds all import/export settings.
	FbxIOSettings* ios = FbxIOSettings::Create(manager, IOSROOT);
	manager->SetIOSettings(ios);

	//Load plugins from the executable directory (optional)
	FbxString lPath = FbxGetApplicationDirectory();
	manager->LoadPluginsDirectory(lPath.Buffer());

	//Create an FBX scene. This object holds most objects imported/exported from/to files.
	pScene = FbxScene::Create(manager, "My Scene");
	if (!pScene)
	{
		FBXSDK_printf("Error: Unable to create FBX scene!\n");
		return 0;
	}
	return 1;
}

bool SaveScene(FbxManager* manager, FbxDocument* scene, const char* name, int fileFormat, bool embedMedia)
{
	int major, minor, revision;
	bool status = true;

	char filename[256];
	sprintf(filename, "%s", name);
	_strlwr(filename);
	char* ptr = strrchr(filename, '.');
	if (ptr && (!strcmp(ptr, ".asf") || !strcmp(ptr, ".amc") || !strcmp(ptr, ".bvh")))
	{
		*ptr = 0;
	}
	strcat(filename, ".fbx");

	// Create an exporter.
	FbxExporter* lExporter = FbxExporter::Create(manager, "");

	if (fileFormat < 0 || fileFormat >= manager->GetIOPluginRegistry()->GetWriterFormatCount())
	{
		// Write in fall back format in less no ASCII format found
		fileFormat = manager->GetIOPluginRegistry()->GetNativeWriterFormat();

		//Try to export in ASCII if possible
		int	lFormatCount = manager->GetIOPluginRegistry()->GetWriterFormatCount();

		for (int lFormatIndex = 0; lFormatIndex < lFormatCount; lFormatIndex++)
		{
			if (manager->GetIOPluginRegistry()->WriterIsFBX(lFormatIndex))
			{
				FbxString lDesc = manager->GetIOPluginRegistry()->GetWriterFormatDescription(lFormatIndex);
				const char *lASCII = "ascii";
				//const char *lASCII = "binary";
				if (lDesc.Find(lASCII) >= 0)
				{
					fileFormat = lFormatIndex;
					break;
				}
			}
		}
	}

	// Set the export states. By default, the export states are always set to 
	// true except for the option eEXPORT_TEXTURE_AS_EMBEDDED. The code below 
	// shows how to change these states.
	IOS_REF.SetBoolProp(EXP_FBX_MATERIAL, true);
	IOS_REF.SetBoolProp(EXP_FBX_TEXTURE, true);
	IOS_REF.SetBoolProp(EXP_FBX_EMBEDDED, embedMedia);
	IOS_REF.SetBoolProp(EXP_FBX_SHAPE, true);
	IOS_REF.SetBoolProp(EXP_FBX_GOBO, true);
	IOS_REF.SetBoolProp(EXP_FBX_ANIMATION, true);
	IOS_REF.SetBoolProp(EXP_FBX_GLOBAL_SETTINGS, true);

	// Initialize the exporter by providing a filename.
	if (lExporter->Initialize(filename, fileFormat, manager->GetIOSettings()) == false)
	{
		FBXSDK_printf("Call to FbxExporter::Initialize() failed.\n");
		FBXSDK_printf("Error returned: %s\n\n", lExporter->GetStatus().GetErrorString());
		return false;
	}

	FbxManager::GetFileFormatVersion(major, minor, revision);
	FBXSDK_printf("FBX file format version %d.%d.%d\n\n", major, minor, revision);

	// Export the scene.
	status = lExporter->Export(scene);

	// Destroy the exporter.
	lExporter->Destroy();
	return status;
}

FbxNode* CreateSkeleton(const exportMeshNode* const model, FbxScene* pScene)
{
	int stack = 1;
	FbxNode* fbxNodesParent[256];
	const exportMeshNode* bvhNodePool[256];
	
	bvhNodePool[0] = model;
	fbxNodesParent[0] = nullptr;

	FbxNode* skeleton = nullptr;
	while (stack)
	{
		stack--;
		const exportMeshNode* const bvhNode = bvhNodePool[stack];
		FbxNode* const fbxParent = fbxNodesParent[stack];

		FbxNode* const fbxNode = FbxNode::Create(pScene, bvhNode->m_name.c_str());
		exportVector posit(bvhNode->m_matrix.m_posit);
		exportVector euler(bvhNode->m_eulers.Scale(180.0f / M_PI));

		fbxNode->LclRotation.Set(FbxVector4(euler.m_x, euler.m_y, euler.m_z));
		fbxNode->LclTranslation.Set(FbxVector4(posit.m_x, posit.m_y, posit.m_z));

		FbxSkeleton* const attribute = FbxSkeleton::Create(pScene, model->m_name.c_str());
		if (fbxParent)
		{
			attribute->Size.Set(0.1f);
			attribute->SetSkeletonType(FbxSkeleton::eLimbNode);
			fbxParent->AddChild(fbxNode);
		}
		else
		{
			attribute->SetSkeletonType(FbxSkeleton::eRoot);
		}
		fbxNode->SetNodeAttribute(attribute);

		if (!skeleton)
		{
			skeleton = fbxNode;
		}

		for (std::list<exportMeshNode*>::const_iterator iter = bvhNode->m_children.begin();
			iter != bvhNode->m_children.end(); iter++)
		{
			bvhNodePool[stack] = *iter;
			fbxNodesParent[stack] = fbxNode;
			stack++;
		}
	}

	return skeleton;
}
bool CreateScene(const exportMeshNode* const model, FbxManager *pSdkManager, FbxScene* pScene)
{
	// create scene info
	FbxDocumentInfo* sceneInfo = FbxDocumentInfo::Create(pSdkManager, "SceneInfo");
	sceneInfo->mTitle = "motion capture skeleton";
	sceneInfo->mSubject = "convert motion capture skeleton";
	sceneInfo->mAuthor = "Newton Dynamics";
	sceneInfo->mRevision = "rev. 1.0";
	sceneInfo->mKeywords = "";
	sceneInfo->mComment = "";

	// we need to add the sceneInfo before calling AddThumbNailToScene because
	// that function is asking the scene for the sceneInfo.
	pScene->SetSceneInfo(sceneInfo);

	//FbxNode* lPatch = CreatePatch(pScene, "Patch");
	FbxNode* lSkeletonRoot = CreateSkeleton(model, pScene);

	// Build the node tree.
	FbxNode* lRootNode = pScene->GetRootNode();
	//lRootNode->AddChild(lPatch);
	lRootNode->AddChild(lSkeletonRoot);

	//// Store poses
	//LinkPatchToSkeleton(pScene, lPatch, lSkeletonRoot);
	//StoreBindPose(pScene, lPatch);
	//StoreRestPose(pScene, lSkeletonRoot);
	//
	//// Animation
	//AnimateSkeleton(pScene, lSkeletonRoot);

	return true;
}
