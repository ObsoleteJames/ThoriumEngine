
#include "ModelCreator.h"
#include "EditorEngine.h"
#include "Game/World.h"
#include "Game/Components/CameraComponent.h"
#include "Game/Components/ModelComponent.h"
#include "Rendering/RenderScene.h"
#include "Resources/Material.h"
#include "Widgets/SaveDialog.h"
#include "Widgets/CollapsableWidget.h"
#include "Widgets/WorldViewportWidget.h"
#include "Widgets/FileDialogs.h"
#include "Widgets/PropertyEditors/ObjectPtrProperty.h"
#include "private/MeshWidget.h"

#include <QLineEdit>
#include <QPushButton>
#include <QPaintEvent>
#include <QCloseEvent>
#include <QWidget>
#include <QBoxLayout>
#include <QMenuBar>
#include <QTreeWidget>
#include <QScrollArea>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

SDK_REGISTER_WINDOW(CModelCreator, "Model Creator", "Tools", NULL);

bool CModelCreator::Shutdown()
{
	if (bRequiresSave && !ShutdownSave())
		return false;

	delete gridMesh;

	SaveState();
	return true;
}

void CModelCreator::SetupUi()
{
	CToolsWindow::SetupUi();

	{
		QMenu* menuFile = new QMenu("File", this);
		QMenu* menuEdit = new QMenu("Edit", this);

		menuFile->addAction("New", this, &CModelCreator::NewModel, QKeySequence(QKeySequence::New));
		menuFile->addAction("Load", this, &CModelCreator::LoadModel, QKeySequence(QKeySequence::Open));
		menuFile->addAction("Save", this, [=]() { this->SaveModel(); }, QKeySequence(QKeySequence::Save));
		menuFile->addAction("Save As", this, [=]() { this->SaveModel(true); }, QKeySequence(QKeySequence::SaveAs));
		
		_menuBar->addMenu(menuFile);
		_menuBar->addMenu(menuEdit);
	}

	setWindowTitle("Model Creator");

	QWidget* widget = new QWidget(this);
	widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setSpacing(0);
	//layout->setMargin(1);
	widget->setLayout(layout);

	setCentralWidget(widget);

	viewport = new CWorldViewportWidget(widget);
	layout->addWidget(viewport);

	{
		detailsWidget = new QDockWidget(this);
		detailsWidget->setWindowTitle("Model");
		detailsWidget->setObjectName("detials_widget");

		QWidget* widget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(widget);
		layout->setMargin(0);
		layout->setSpacing(0);
		detailsWidget->setWidget(widget);

		QScrollArea* scrollArea = new QScrollArea(this);
		scrollArea->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
		scrollArea->setWidgetResizable(true);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		layout->addWidget(scrollArea);

		QFrame* scrollWidget = new QFrame(this);
		QVBoxLayout* l2 = new QVBoxLayout(scrollWidget);
		scrollWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum));
		l2->setMargin(0);
		l2->setSpacing(0);
		scrollArea->setWidget(scrollWidget);
		
		QPushButton* btnCompile = new QPushButton("Compile", this);
		connect(btnCompile, &QPushButton::clicked, this, [=]() { Compile(); });

		l2->addWidget(btnCompile);
		l2->addSpacing(6);

		meshesWidget = new QWidget(this);
		QVBoxLayout* meshLayout = new QVBoxLayout(meshesWidget);
		meshLayout->setMargin(0);
		meshLayout->setSpacing(0);

		meshList = new CCollapsableWidget("Meshes", meshesWidget, this);
		l2->addWidget(meshList);

		QHBoxLayout* l3 = new QHBoxLayout(meshList->GetHeader());
		l3->setMargin(2);
		l3->addStretch(1);
		QPushButton* addMeshBtn = new QPushButton("+", meshList);
		addMeshBtn->setProperty("type", QVariant("clear"));
		l3->addWidget(addMeshBtn);

		materialsWidget = new QWidget(this);
		QVBoxLayout* matLayout = new QVBoxLayout(materialsWidget);
		matLayout->setMargin(0);
		matLayout->setSpacing(0);

		materialsList = new CCollapsableWidget("Materials", materialsWidget, this);
		l2->addWidget(materialsList);
		
		connect(addMeshBtn, &QPushButton::clicked, this, &CModelCreator::AddMesh);

		addDockWidget(Qt::LeftDockWidgetArea, detailsWidget);
	}

	{
		skeletonWidget = new QDockWidget(this);
		skeletonWidget->setWindowTitle("Skeleton");
		skeletonWidget->setObjectName("skeleton_widget");

		QWidget* widget = new QWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(widget);
		layout->setMargin(0);
		layout->setSpacing(0);
		skeletonWidget->setWidget(widget);

		skeletonTree = new QTreeWidget(widget);

		layout->addWidget(skeletonTree);

		addDockWidget(Qt::LeftDockWidgetArea, skeletonWidget);
	}

	RestoreState();
	UpdateUI();

	Init();
}

void CModelCreator::Init()
{
	world = CreateObject<CWorld>();
	world->InitWorld(CWorld::InitializeInfo().CreateAISystems(false).CreatePhyiscsWorld(false).RegisterForRendering(false));
	//world->SetRenderScene(viewport->GetRenderScene());
	viewport->SetOverrideScene(world->GetRenderScene());

	camera = CreateObject<CCameraComponent>();
	viewport->SetControlMode(ECameraControlMode::Orbit);
	viewport->camera = camera;
	world->GetRenderScene()->SetCamera(camera);

	TObjectPtr<CEntity> modelEnt = world->CreateEntity<CEntity>();
	modelComp = modelEnt->AddComponent<CModelComponent>("Model");

	gridMesh = new FMesh();
	gEditorEngine()->GenerateGrid(10.f, 1.f, gridMesh);

	gridMat = CreateObject<CMaterial>();
	gridMat->SetShader("Tools");
	gridMat->SetInt("vType", 1);
}

void CModelCreator::UserSaveState(QSettings& settings)
{

}

void CModelCreator::UserRestoreState(QSettings& settings)
{

}

void CModelCreator::SetModel(CModelAsset* m, bool bNew)
{
	if (bRequiresSave && model)
	{
		if (!ShutdownSave())
			return;
	}

	data.Clear();
	bReadOnly = false;

	ClearMeshList();

	// TODO: Load SDK Data
	if (!bNew)
	{
		CFStream sdkStream = model->File()->GetSdkStream("rb");
		if (sdkStream.IsOpen())
		{
			SizeType numMeshes;
			sdkStream >> &numMeshes;

			for (SizeType i = 0ull; i < numMeshes; i++)
			{
				FImportedMesh mesh;
				sdkStream >> mesh.file;
				sdkStream >> mesh.name;
				sdkStream >> &mesh.position;
				sdkStream >> &mesh.rotation;
				sdkStream >> &mesh.scale;

				SizeType numObjects;
				sdkStream >> &numObjects;

				for (SizeType x = 0ull; x < numObjects; x++)
				{
					FString n;
					bool b;

					sdkStream >> n;
					sdkStream >> &b;

					mesh.objects[n.c_str()] = b;
				}

				data.importedMeshes.Add(mesh);

				CMeshWidget* mw = new CMeshWidget(this, &*data.importedMeshes.last(), meshWidgets.Size());
				meshWidgets.Add(mw);
				meshesWidget->layout()->addWidget(mw);
			}
		}
		else
			bReadOnly = true;
	}

	model = m;
	modelComp->SetModel(model);
	UpdateUI();
	UpdateMaterials();
	UpdateSkeleton();
}

void CModelCreator::paintEvent(QPaintEvent* event)
{
	world->Update(viewport->GetDeltaTime());
	world->Render();

	{
		FDrawMeshCmd cmd;
		cmd.material = gridMat;
		cmd.mesh = gridMesh;
		cmd.transform = FMatrix(1.f);
		cmd.drawType |= MESH_DRAW_PRIMITIVE_LINES;

		FRenderCommand gridDraw(cmd, R_DEBUG_PASS);
		world->GetRenderScene()->PushCommand(gridDraw);
	}

	viewport->Render();

	if (isActiveWindow())
		update(viewport->rect());

	event->accept();
}

void CModelCreator::closeEvent(QCloseEvent* event)
{
	if (!CloseWindow())
	{
		event->ignore();
		return;
	}

	world->Delete();
	world = nullptr;

	event->accept();
}

bool CModelCreator::ShutdownSave()
{
	CSaveDialog* saveDialog = new CSaveDialog(this);
	int r = saveDialog->exec();

	if (r == CSaveDialog::SAVE_SUCCESS)
	{
		SaveModel();
		return true;
	}
	
	return r;
}

void CModelCreator::UpdateUI()
{
	if (!model)
	{
		detailsWidget->widget()->setDisabled(true);
		setWindowTitle("Model Creator");
		return;
	}

	detailsWidget->widget()->setDisabled(bReadOnly);

	UpdateTitle();
}

void CModelCreator::Compile()
{
	model->bodyGroups.Clear();
	model->numLODs = 0;
	model->ClearMeshes();
	//model->materials.Clear();
	model->skeleton.bones.Clear();

	SizeType numMats = 0;
	SizeType numBones = 0;
	for (FImportedMesh& mesh : data.importedMeshes)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(ToFString(mesh.file).c_str(), aiProcess_Triangulate | aiProcess_MakeLeftHanded | aiProcess_CalcTangentSpace | aiProcess_OptimizeMeshes | aiProcess_FlipUVs | aiProcess_PopulateArmatureData);
		if (!scene)
			continue;

		SizeType numMeshes = model->meshes.Size();

		FQuaternion meshRotate = FQuaternion::EulerAngles(mesh.rotation.Radians());

		model->materials.Resize(scene->mNumMaterials + numMats);

		for (int i = 0; i < scene->mNumMaterials; i++)
		{
			if (model->materials[i + numMats].name == scene->mMaterials[i]->GetName().C_Str())
				continue;

			FMaterial& m = model->materials[i + numMats];
			m.name = scene->mMaterials[i]->GetName().C_Str();
			m.path = L"";
			m.obj = nullptr;

			//model->materials.Add({ scene->mMaterials[i]->GetName().C_Str(), L"", nullptr });
		}
		numMats += scene->mNumMaterials;

		TMap<aiNode*, SizeType> boneIndices;

		for (int i = 0; i < scene->mNumSkeletons; i++)
		{
			aiSkeleton* skel = scene->mSkeletons[i];

			for (int b = 0; b < skel->mNumBones; b++)
			{
				aiSkeletonBone* bone = skel->mBones[b];
				boneIndices[bone->mNode] = model->skeleton.bones.Size();

				FBone _bone{};
				_bone.name = bone->mNode->mName.C_Str();
				_bone.parent = bone->mParent;
				if (_bone.parent != -1)
					_bone.parent += numBones;

				aiVector3D bonePos;
				aiQuaternion boneRot;
				aiVector3D boneDir = boneRot.Rotate({ 0, 0, -1 });
				bone->mOffsetMatrix.DecomposeNoScaling(boneRot, bonePos);

				_bone.position = *(FVector*)&bonePos;
				_bone.direction = *(FVector*)&boneDir;

				model->skeleton.bones.Add(_bone);
			}
			numBones += skel->mNumBones;
		}
		
		for (int i = 0; i < scene->mNumMeshes; i++)
			CompileMesh(scene, scene->mMeshes[i], mesh.position, meshRotate, mesh.scale);

		boneIndices.clear();
	}

	bRequiresSave = true;
	modelComp->SetModel(model);
	UpdateMaterials();
	UpdateSkeleton();
	UpdateTitle();
}

void CModelCreator::CompileMesh(const aiScene* scene, const aiMesh* importMesh, const FVector& offset, const FQuaternion& rotation, const FVector& scale)
{
	model->meshes.Add();
	FMesh& m = *model->meshes.last();

	for (SizeType mat = 0; mat < model->materials.Size(); mat++)
	{
		if (model->materials[mat].name == scene->mMaterials[importMesh->mMaterialIndex]->GetName().C_Str())
		{
			m.materialIndex = mat;
			break;
		}
	}

	m.numVertices = importMesh->mNumVertices;
	m.numIndices = importMesh->mNumFaces * 3;
	m.numVertexData = m.numVertices;
	m.numIndexData = m.numIndices;
	m.vertexData = new FVertex[m.numVertexData];
	m.indexData = new uint[m.numIndexData];

	m.meshName = importMesh->mName.C_Str();

	for (SizeType v = 0; v < m.numVertexData; v++)
	{
		FVertex vert{};
		vert.position = *(FVector*)&importMesh->mVertices[v];
		vert.normal = *(FVector*)&importMesh->mNormals[v];
		if (importMesh->mTangents)
			vert.tangent = *(FVector*)&importMesh->mTangents[v];

		if (importMesh->HasVertexColors(v))
			vert.color = *(FVector*)&importMesh->mColors[v];

		vert.position = rotation.Rotate(vert.position * scale) + offset;
		vert.normal = rotation.Rotate(vert.normal);

		for (int b = 0; b < 4; b++)
			vert.bones[b] = -1;

		if (importMesh->GetNumUVChannels() > 0)
		{
			vert.uv1[0] = importMesh->mTextureCoords[0][v].x;
			vert.uv1[1] = importMesh->mTextureCoords[0][v].y;
		}
		if (importMesh->GetNumUVChannels() > 1)
		{
			vert.uv2[0] = importMesh->mTextureCoords[1][v].x;
			vert.uv2[1] = importMesh->mTextureCoords[1][v].y;
		}
		m.vertexData[v] = vert;
	}
	for (int f = 0; f < importMesh->mNumFaces; f++)
	{
		m.indexData[f * 3] = importMesh->mFaces[f].mIndices[0];
		m.indexData[f * 3 + 1] = importMesh->mFaces[f].mIndices[2];
		m.indexData[f * 3 + 2] = importMesh->mFaces[f].mIndices[1];
	}

	{
		TArray<FVertex> vertBuf;
		vertBuf.Resize(m.numVertices);
		memcpy(vertBuf.Data(), m.vertexData, m.numVertices * sizeof(FVertex));
		m.vertexBuffer = gRenderer->CreateVertexBuffer(vertBuf);
	}
	{
		TArray<uint> indexBuf;
		indexBuf.Resize(m.numIndices);
		memcpy(indexBuf.Data(), m.indexData, m.numIndices * sizeof(uint));
		m.indexBuffer = gRenderer->CreateIndexBuffer(indexBuf);
	}

	//if (boneIndices.size() > 0)
	//{
	//	for (int b = 0; b < importMesh->mNumBones; b++)
	//	{
	//		aiBone* importBone = importMesh->mBones[b];

	//		for (int w = 0; w < importBone->mNumWeights; w++)
	//		{
	//			aiVertexWeight& weight = importBone->mWeights[w];
	//			FVertex& v = m.vertexData[weight.mVertexId];
	//			v.bones[w] = boneIndices[importBone->mNode];
	//			v.boneInfluence[w] = weight.mWeight;
	//		}
	//	}
	//}
}

void CModelCreator::UpdateMaterials()
{
	for (auto* mat : materialWidgets)
		materialsWidget->layout()->removeWidget(mat);

	materialWidgets.Clear();

	int index = 0;
	for (auto& mat : model->GetMaterials())
	{
		void* matPtr = (void*)&model->GetMaterials()[index].obj;
		CObjectPtrProperty* p = new CObjectPtrProperty(mat.name, matPtr, CMaterial::StaticClass(), materialsWidget);
		materialsWidget->layout()->addWidget(p);
		materialWidgets.Add(p);

		connect(p, &CObjectPtrProperty::OnValueChanged, this, [=]() {
			CMaterial* m = modelComp->GetMaterial(index);
			if (!m)
				((FMaterial*)(&model->GetMaterials()[index]))->path = WString();
			else
				((FMaterial*)(&model->GetMaterials()[index]))->path = m->GetPath();

			bRequiresSave = true;
			UpdateTitle();
		});

		index++;
	}
}

void CModelCreator::UpdateSkeleton()
{
	TArray<QTreeWidgetItem*> bones;
	skeletonTree->clear();

	if (!model)
		return;

	for (auto& b : model->skeleton.bones)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem();
		item->setText(0, b.name.c_str());

		if (b.parent != -1)
			bones[b.parent]->addChild(item);
		else
			skeletonTree->addTopLevelItem(item);
	}
}

void CModelCreator::AddMesh()
{
	if (bReadOnly)
		return;

	data.importedMeshes.Add();
	for (SizeType i = 0; i < meshWidgets.Size(); i++)
		meshWidgets[i]->SetData(&data.importedMeshes[i]);

	CMeshWidget* mw = new CMeshWidget(this, &*data.importedMeshes.last(), meshWidgets.Size());
	meshWidgets.Add(mw);
	meshesWidget->layout()->addWidget(mw);
	
	MarkDirty();
}

void CModelCreator::RemoveMesh(SizeType id)
{
	data.importedMeshes.Erase(data.importedMeshes.At(id));

	CMeshWidget* mw = *meshWidgets.last();
	meshesWidget->layout()->removeWidget(mw);
	delete mw;
	meshWidgets.Erase(meshWidgets.last());

	for (SizeType i = 0; i < meshWidgets.Size(); i++)
	{
		meshWidgets[i]->id = i;
		meshWidgets[i]->SetData(&data.importedMeshes[i]);
	}

	MarkDirty();
}

void CModelCreator::UpdateMesh(SizeType id)
{
	if (!(id < data.importedMeshes.Size()))
		return;

	FImportedMesh& mesh = data.importedMeshes[id];
	if (mesh.file.IsEmpty())
		return;

	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile(ToFString(mesh.file).c_str(), 0);
	if (!scene)
		return;

	if (auto* node = scene->mRootNode)
	{
		for (uint i = 0; i < node->mNumChildren; i++)
		{
			auto* child = node->mChildren[i];
			auto it = mesh.objects.find(child->mName.C_Str());
			if (it != mesh.objects.end())
			{
				mesh.objects.emplace(child->mName.C_Str(), true);
			}
		}
	}
	Compile();
}

void CModelCreator::NewModel()
{
	if (bRequiresSave && model)
	{
		if (!ShutdownSave())
			return;
	}

	model = CreateObject<CModelAsset>();
	model->bInitialized = true;
	SetModel(model, true);
}

void CModelCreator::LoadModel()
{
	if (bRequiresSave && model)
	{
		if (!ShutdownSave())
			return;
	}

	COpenFileDialog dialog((FAssetClass*)CModelAsset::StaticClass(), this);
	if (!dialog.exec())
		return;

	model = CResourceManager::GetResource<CModelAsset>(dialog.File()->Path());
	SetModel(model);
}

void CModelCreator::SaveModel(bool bNewPath /*= false*/)
{
	if (bReadOnly || !model)
		return;

	if (!model->File())
	{
		CSaveFileDialog* dialog = new CSaveFileDialog(this);
		if (!dialog->exec())
			return;

		CResourceManager::RegisterNewResource(model, dialog->Path());
	}
	else if (bNewPath)
	{
		CSaveFileDialog* dialog = new CSaveFileDialog(this);
		if (!dialog->exec())
			return;

		// TODO: Save As
		return;
	}

	// Save SDK Data
	CFStream sdkStream = model->File()->GetSdkStream("wb");
	if (sdkStream.IsOpen())
	{
		SizeType numMeshes = data.importedMeshes.Size();
		sdkStream << &numMeshes;
		
		for (auto& m : data.importedMeshes)
		{
			sdkStream << m.file;
			sdkStream << m.name;
			sdkStream << &m.position;
			sdkStream << &m.rotation;
			sdkStream << &m.scale;

			SizeType numObjects = m.objects.size();

			sdkStream << &numObjects;

			for (auto& obj : m.objects)
			{
				sdkStream << FString(obj.first.c_str());
				sdkStream << &obj.second;
			}
		}
	}

	model->Save();
	bRequiresSave = false;
	UpdateTitle();
}

void CModelCreator::UpdateTitle()
{
	if (!model)
	{
		setWindowTitle("Model Creator");
		return;
	}

	if (model->File())
	{
		FFile* f = model->File();
		
		WString title = L"Model Creator - " + f->Name();
		if (bRequiresSave)
			title += L'*';
		if (bReadOnly)
			title += L" (Read Only)";

		setWindowTitle(QString((const QChar*)title.c_str()));
	}
	else
	{
		WString title = L"Model Creator - New Model";
		if (bRequiresSave)
			title += L'*';

		setWindowTitle(QString((const QChar*)title.c_str()));
	}
}

void CModelCreator::ClearMeshList()
{
	for (auto* m : meshWidgets)
		meshesWidget->layout()->removeWidget(m);
	
	meshWidgets.Clear();
}
