#include "Octree.h"
using namespace Simplex;

uint MyOctant::m_uOctantCount = 0;
uint MyOctant::m_uMaxLevel = 3;
uint MyOctant::m_uIdealEntityCount = 5;
uint MyOctant::GetOctantCount(void) { return m_uOctantCount; }
void MyOctant::Init(void)
{
	m_uChildren = 0;

	m_fSize = 0;

	m_uID = m_uOctantCount;
	m_uLevel = 0;

	m_v3Center = vector3(0.0f);
	m_v3Min = vector3(0.0f);
	m_v3Max = vector3(0.0f);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	m_pRoot = nullptr;
	m_pParent = nullptr;
	for (uint n = 0; n < 8; n++) 
	{
		m_pChild[n] = nullptr;
	}
}
void MyOctant::Swap(MyOctant& other) 
{
	std::swap(m_uChildren, other.m_uChildren);

	std::swap(m_fSize, other.m_fSize);
	std::swap(m_uID, other.m_uID);
	std::swap(m_pRoot, other.m_pRoot);
	std::swap(m_lChild, other.m_lChild);

	std::swap(m_v3Center, other.m_v3Center);
	std::swap(m_v3Min, other.m_v3Min);
	std::swap(m_v3Max, other.m_v3Max);

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	std::swap(m_uLevel, other.m_uLevel);
	std::swap(m_pParent, other.m_pParent);
	for (uint i = 0; i < 8; i++) 
	{
		std::swap(m_pChild[i], other.m_pChild[i]);
	}
}
MyOctant * MyOctant::GetParent(void) { return m_pParent; }
void MyOctant::Release(void) 
{
	//special kind of release only for root
	if (m_uLevel == 0) 
	{
		KillBranches();
	}
	m_uChildren = 0;
	m_fSize = 0.0f;
	m_EntityList.clear();
	m_lChild.clear();
}
//the big 3
MyOctant::MyOctant(uint a_nMaxLevel, uint a_nIdealEntityCount) 
{
	//for default values
	Init();

	m_uOctantCount = 0;
	m_uMaxLevel = a_nMaxLevel;
	m_uIdealEntityCount = a_nIdealEntityCount;
	m_uID = m_uOctantCount;

	m_pRoot = this;
	m_lChild.clear();

	std::vector<vector3> lMinMax; //holds all of the max and min vectors of the bounding object

	uint nObjects = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < nObjects; i++) 
	{
		MyEntity* pEntity = m_pEntityMngr->GetEntity(i);
		MyRigidBody* pRigidBody = pEntity->GetRigidBody();
		lMinMax.push_back(pRigidBody->GetMinGlobal());
		lMinMax.push_back(pRigidBody->GetMaxGlobal());
	}
	MyRigidBody* pRigidBody = new MyRigidBody(lMinMax);

	vector3 vHalfWidth = pRigidBody->GetHalfWidth();
	float fMax = vHalfWidth.x;
	for (int i = 0; i < 3; i++) 
	{
		if (fMax < vHalfWidth[i])
			fMax = vHalfWidth[i];
	}
	vector3 v3Center = pRigidBody->GetCenterLocal();
	lMinMax.clear();
	SafeDelete(pRigidBody);

	m_fSize = fMax * 2.0f;
	m_v3Center = v3Center;
	m_v3Min = m_v3Center - (vector3(fMax));
	m_v3Max = m_v3Center + (vector3(fMax));

	m_uOctantCount++;

	ConstructTree(m_uMaxLevel);
}

MyOctant::MyOctant(vector3 a_v3Center, float a_fSize) 
{
	//for default vaules
	Init();
	m_v3Center = a_v3Center;
	m_fSize = a_fSize;

	m_v3Min = m_v3Center - (vector3(m_fSize) / 2.0f);
	m_v3Max = m_v3Center + (vector3(m_fSize) / 2.0f);

	m_uOctantCount++;
}


MyOctant::MyOctant(MyOctant const& other) 
{
	m_uChildren = other.m_uChildren;
	m_v3Center = other.m_v3Center;
	m_v3Min = other.m_v3Min;
	m_v3Max = other.m_v3Max;

	m_fSize = other.m_fSize;
	m_uID = other.m_uID;
	m_uLevel = other.m_uLevel;
	m_pParent = other.m_pParent;

	m_pRoot, other.m_pRoot;
	m_lChild, other.m_lChild;

	m_pMeshMngr = MeshManager::GetInstance();
	m_pEntityMngr = MyEntityManager::GetInstance();

	for (uint i = 0; i < 8; i++)
	{
		m_pChild[i] = other.m_pChild[i];
	}
}

MyOctant& MyOctant::operator=(MyOctant const& other) 
{
	if (this != &other)
	{
		Release();
		Init();
		MyOctant temp(other);
		Swap(temp);
	}
	return *this;
}
MyOctant::~MyOctant() { Release(); }

//Accessors
float MyOctant::GetSize(void) { return m_fSize; }
vector3 MyOctant::GetCenterGlobal(void) { return m_v3Center; }
vector3 MyOctant::GetMinGlobal(void) { return m_v3Min; }
vector3 MyOctant::GetMaxGlobal(void) { return m_v3Max; }

void MyOctant::Display(uint a_nIndex, vector3 a_v3Color)
{
	if (m_uID == a_nIndex)
	{
		m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
		return;
	}

	for (uint i = 0; i < m_uChildren; i++) 
	{
		m_pChild[i]->Display(a_nIndex);
	}
}

void MyOctant::Display(vector3 a_v3Color) 
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->Display(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}

void MyOctant::Subdivide(void) 
{
	//if maximum depth reached, do nothing
	if (m_uLevel >= m_uMaxLevel) 
	{
		return;
	}

	//if it's already been divided
	if (m_uChildren != 0)
	{
		return;
	}

	//not a leaf
	m_uChildren = 8;

	float fSize = m_fSize / 4.0f;
	float fSizeD = fSize * 2.0f;
	vector3 v3Center;

	//bottom left back
	v3Center = m_v3Center;
	v3Center.x -= fSize;
	v3Center.y -= fSize;
	v3Center.z -= fSize;
	m_pChild[0] = new MyOctant(v3Center, fSizeD);

	//bottom right back
	v3Center.x += fSizeD;
	m_pChild[1] = new MyOctant(v3Center, fSizeD);

	//bottom right front
	v3Center.z += fSizeD;
	m_pChild[2] = new MyOctant(v3Center, fSizeD);

	//bottom left front
	v3Center.x -= fSizeD;
	m_pChild[3] = new MyOctant(v3Center, fSizeD);

	//top left front
	v3Center.y += fSizeD;
	m_pChild[4] = new MyOctant(v3Center, fSizeD);

	//top left back
	v3Center.z -= fSizeD;
	m_pChild[5] = new MyOctant(v3Center, fSizeD);

	//top right back
	v3Center.x += fSizeD;
	m_pChild[6] = new MyOctant(v3Center, fSizeD);

	//top right front
	v3Center.z += fSizeD;
	m_pChild[7] = new MyOctant(v3Center, fSizeD);

	for (uint i = 0; i < 8; i++) 
	{
		m_pChild[i]->m_pRoot = m_pRoot;
		m_pChild[i]->m_pParent = this;
		m_pChild[i]->m_uLevel = m_uLevel + 1;
		if (m_pChild[i]->ContainsMoreThan(m_uIdealEntityCount))
		{
			m_pChild[i]->Subdivide();
		}
	}
}
MyOctant * MyOctant::GetChild(uint a_nChild)
{
	if (a_nChild > 7) return nullptr;
	return m_pChild[a_nChild];
}
bool MyOctant::IsColliding(uint a_uRBIndex)
{
	uint nObjectCount = m_pEntityMngr->GetEntityCount();

	if (a_uRBIndex >= nObjectCount)
		return false;

	MyEntity* pEntity = m_pEntityMngr->GetEntity(a_uRBIndex);
	MyRigidBody* pRigidBody = pEntity->GetRigidBody();
	vector3 v3MinD = pRigidBody->GetMinGlobal();
	vector3 v3MaxD = pRigidBody->GetMinGlobal();

	if (m_v3Max.x < v3MinD.x)
		return false;
	if (m_v3Min.x > v3MaxD.x)
		return false;

	if (m_v3Max.y < v3MinD.y)
		return false;
	if (m_v3Min.y > v3MaxD.y)
		return false;

	if (m_v3Max.z < v3MinD.z)
		return false;
	if (m_v3Min.z > v3MaxD.z)
		return false;

	return true;
}
bool MyOctant::IsLeaf(void) { return m_uChildren == 0; }
bool MyOctant::ContainsMoreThan(uint a_nEntities) 
{
	uint nCount = 0;
	uint nObjectCount = m_pEntityMngr->GetEntityCount();
	for (uint i = 0; i < nObjectCount; i++) 
	{
		if (IsColliding(i)) 
		{
			nCount++;
		}
		if (nCount > a_nEntities) 
		{
			return true;
		}
	}
	return false;
}
void MyOctant::KillBranches(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->KillBranches();
		delete m_pChild[i];
		m_pChild[i] = nullptr;
	}
	m_uChildren = 0;
}
void MyOctant::DisplayLeafs(vector3 a_v3Color)
{
	uint nLeafs = m_lChild.size();
	for (uint i = 0; i < nLeafs; i++)
	{
		m_lChild[i]->DisplayLeafs(a_v3Color);
	}
	m_pMeshMngr->AddWireCubeToRenderList(glm::translate(IDENTITY_M4, m_v3Center) * glm::scale(vector3(m_fSize)), a_v3Color, RENDER_WIRE);
}
void MyOctant::ClearEntityList(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ClearEntityList();
	}
	m_EntityList.clear();
}
void MyOctant::ConstructTree(uint a_nMaxLevel) 
{
	//if not root, do nothing
	if (m_uLevel != 0)
	{
		return;
	}
	m_uMaxLevel = a_nMaxLevel;
	m_uOctantCount = 1;
	m_EntityList.clear();

	KillBranches();
	m_lChild.clear();

	if (ContainsMoreThan(m_uIdealEntityCount)) 
	{
		Subdivide();
	}

	AssignIDtoEntity();

	ConstructList();
}
void MyOctant::AssignIDtoEntity(void)
{
	//go until you get to a leaf
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->AssignIDtoEntity();
	}
	if (m_uChildren == 0) 
	{
		uint nEntities = m_pEntityMngr->GetEntityCount();
		for (uint i = 0; i < nEntities; i++) 
		{
			if (IsColliding(i))
			{
				m_EntityList.push_back(i);
				m_pEntityMngr->AddDimension(i, m_uID);
			}
		}
	}
}
void MyOctant::ConstructList(void)
{
	for (uint i = 0; i < m_uChildren; i++)
	{
		m_pChild[i]->ConstructList();
	}
	if (m_EntityList.size() > 0)
	{
		m_pRoot->m_lChild.push_back(this);
	}
}