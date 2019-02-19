#include "MyMesh.h"
void MyMesh::Init(void)
{
	m_bBinded = false;
	m_uVertexCount = 0;

	m_VAO = 0;
	m_VBO = 0;

	m_pShaderMngr = ShaderManager::GetInstance();
}
void MyMesh::Release(void)
{
	m_pShaderMngr = nullptr;

	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);

	if (m_VAO > 0)
		glDeleteVertexArrays(1, &m_VAO);

	m_lVertex.clear();
	m_lVertexPos.clear();
	m_lVertexCol.clear();
}
MyMesh::MyMesh()
{
	Init();
}
MyMesh::~MyMesh() { Release(); }
MyMesh::MyMesh(MyMesh& other)
{
	m_bBinded = other.m_bBinded;

	m_pShaderMngr = other.m_pShaderMngr;

	m_uVertexCount = other.m_uVertexCount;

	m_VAO = other.m_VAO;
	m_VBO = other.m_VBO;
}
MyMesh& MyMesh::operator=(MyMesh& other)
{
	if (this != &other)
	{
		Release();
		Init();
		MyMesh temp(other);
		Swap(temp);
	}
	return *this;
}
void MyMesh::Swap(MyMesh& other)
{
	std::swap(m_bBinded, other.m_bBinded);
	std::swap(m_uVertexCount, other.m_uVertexCount);

	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);

	std::swap(m_lVertex, other.m_lVertex);
	std::swap(m_lVertexPos, other.m_lVertexPos);
	std::swap(m_lVertexCol, other.m_lVertexCol);

	std::swap(m_pShaderMngr, other.m_pShaderMngr);
}
void MyMesh::CompleteMesh(vector3 a_v3Color)
{
	uint uColorCount = m_lVertexCol.size();
	for (uint i = uColorCount; i < m_uVertexCount; ++i)
	{
		m_lVertexCol.push_back(a_v3Color);
	}
}
void MyMesh::AddVertexPosition(vector3 a_v3Input)
{
	m_lVertexPos.push_back(a_v3Input);
	m_uVertexCount = m_lVertexPos.size();
}
void MyMesh::AddVertexColor(vector3 a_v3Input)
{
	m_lVertexCol.push_back(a_v3Input);
}
void MyMesh::CompileOpenGL3X(void)
{
	if (m_bBinded)
		return;

	if (m_uVertexCount == 0)
		return;

	CompleteMesh();

	for (uint i = 0; i < m_uVertexCount; i++)
	{
		//Position
		m_lVertex.push_back(m_lVertexPos[i]);
		//Color
		m_lVertex.push_back(m_lVertexCol[i]);
	}
	glGenVertexArrays(1, &m_VAO);//Generate vertex array object
	glGenBuffers(1, &m_VBO);//Generate Vertex Buffered Object

	glBindVertexArray(m_VAO);//Bind the VAO
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);//Bind the VBO
	glBufferData(GL_ARRAY_BUFFER, m_uVertexCount * 2 * sizeof(vector3), &m_lVertex[0], GL_STATIC_DRAW);//Generate space for the VBO

																									   // Position attribute
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)0);

	// Color attribute
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 2 * sizeof(vector3), (GLvoid*)(1 * sizeof(vector3)));

	m_bBinded = true;

	glBindVertexArray(0); // Unbind VAO
}
void MyMesh::Render(matrix4 a_mProjection, matrix4 a_mView, matrix4 a_mModel)
{
	// Use the buffer and shader
	GLuint nShader = m_pShaderMngr->GetShaderID("Basic");
	glUseProgram(nShader);

	//Bind the VAO of this object
	glBindVertexArray(m_VAO);

	// Get the GPU variables by their name and hook them to CPU variables
	GLuint MVP = glGetUniformLocation(nShader, "MVP");
	GLuint wire = glGetUniformLocation(nShader, "wire");

	//Final Projection of the Camera
	matrix4 m4MVP = a_mProjection * a_mView * a_mModel;
	glUniformMatrix4fv(MVP, 1, GL_FALSE, glm::value_ptr(m4MVP));

	//Solid
	glUniform3f(wire, -1.0f, -1.0f, -1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);

	//Wire
	glUniform3f(wire, 1.0f, 0.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_POLYGON_OFFSET_LINE);
	glPolygonOffset(-1.f, -1.f);
	glDrawArrays(GL_TRIANGLES, 0, m_uVertexCount);
	glDisable(GL_POLYGON_OFFSET_LINE);

	glBindVertexArray(0);// Unbind VAO so it does not get in the way of other objects
}
void MyMesh::AddTri(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft)
{
	//C
	//| \
		//A--B
//This will make the triangle A->B->C 
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);
}
void MyMesh::AddQuad(vector3 a_vBottomLeft, vector3 a_vBottomRight, vector3 a_vTopLeft, vector3 a_vTopRight)
{
	//C--D
	//|  |
	//A--B
	//This will make the triangle A->B->C and then the triangle C->B->D
	AddVertexPosition(a_vBottomLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopLeft);

	AddVertexPosition(a_vTopLeft);
	AddVertexPosition(a_vBottomRight);
	AddVertexPosition(a_vTopRight);
}
void MyMesh::GenerateCube(float a_fSize, vector3 a_v3Color)
{
	if (a_fSize < 0.01f)
		a_fSize = 0.01f;

	Release();
	Init();

	float fValue = a_fSize * 0.5f;
	//3--2
	//|  |
	//0--1

	vector3 point0(-fValue, -fValue, fValue); //0
	vector3 point1(fValue, -fValue, fValue); //1
	vector3 point2(fValue, fValue, fValue); //2
	vector3 point3(-fValue, fValue, fValue); //3

	vector3 point4(-fValue, -fValue, -fValue); //4
	vector3 point5(fValue, -fValue, -fValue); //5
	vector3 point6(fValue, fValue, -fValue); //6
	vector3 point7(-fValue, fValue, -fValue); //7

											  //F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCuboid(vector3 a_v3Dimensions, vector3 a_v3Color)
{
	Release();
	Init();

	vector3 v3Value = a_v3Dimensions * 0.5f;
	//3--2
	//|  |
	//0--1
	vector3 point0(-v3Value.x, -v3Value.y, v3Value.z); //0
	vector3 point1(v3Value.x, -v3Value.y, v3Value.z); //1
	vector3 point2(v3Value.x, v3Value.y, v3Value.z); //2
	vector3 point3(-v3Value.x, v3Value.y, v3Value.z); //3

	vector3 point4(-v3Value.x, -v3Value.y, -v3Value.z); //4
	vector3 point5(v3Value.x, -v3Value.y, -v3Value.z); //5
	vector3 point6(v3Value.x, v3Value.y, -v3Value.z); //6
	vector3 point7(-v3Value.x, v3Value.y, -v3Value.z); //7

													   //F
	AddQuad(point0, point1, point3, point2);

	//B
	AddQuad(point5, point4, point6, point7);

	//L
	AddQuad(point4, point0, point7, point3);

	//R
	AddQuad(point1, point5, point2, point6);

	//U
	AddQuad(point3, point2, point7, point6);

	//D
	AddQuad(point4, point5, point0, point1);

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCone(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	std::vector<vector3> pointsList;

	vector3 point0(0, a_fHeight / 2, 0);
	vector3 pointBottom(0, -a_fHeight * 0.5, 0);

	for (int i = 0; i < a_nSubdivisions; i++) {
		vector3 point(a_fRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (-a_fHeight)*0.5, a_fRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));

		pointsList.push_back(point);
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		if (i == a_nSubdivisions - 1) {
			AddTri(pointsList[i], pointsList[0], point0);
			AddTri(pointsList[0], pointsList[i], pointBottom);
		}
		else {
			AddTri(pointsList[i], pointsList[i + 1], point0);
			AddTri(pointsList[i + 1], pointsList[i], pointBottom);
		}

	}
	//AddTri(point2, point3, point0);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateCylinder(float a_fRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	std::vector<vector3> topPoints;
	std::vector<vector3> bottomPoints;

	vector3 pointTop(0, a_fHeight * 0.5, 0);
	vector3 pointBottom(0, -a_fHeight * 0.5, 0);

	for (int i = 0; i < a_nSubdivisions; i++) {
		vector3 pointB(a_fRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (-a_fHeight)*0.5, a_fRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));
		bottomPoints.push_back(pointB);

		vector3 pointA(a_fRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (a_fHeight)*0.5, a_fRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));
		topPoints.push_back(pointA);
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		if (i == a_nSubdivisions - 1) {
			AddTri(topPoints[i], topPoints[0], pointTop);
			AddTri(bottomPoints[0], bottomPoints[i], pointBottom);
			AddQuad(bottomPoints[i], bottomPoints[0], topPoints[i], topPoints[0]);
		}
		else {
			AddTri(topPoints[i], topPoints[i + 1], pointTop);
			AddTri(bottomPoints[i + 1], bottomPoints[i], pointBottom);
			AddQuad(bottomPoints[i], bottomPoints[i + 1], topPoints[i], topPoints[i + 1]);
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTube(float a_fOuterRadius, float a_fInnerRadius, float a_fHeight, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_fHeight < 0.01f)
		a_fHeight = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	Release();
	Init();

	// Replace this with your code
	std::vector<vector3> topOuterPoints;
	std::vector<vector3> topInnerPoints;
	std::vector<vector3> bottomOuterPoints;
	std::vector<vector3> bottomInnerPoints;

	vector3 pointTop(0, a_fHeight * 0.5, 0);
	vector3 pointBottom(0, -a_fHeight * 0.5, 0);

	for (int i = 0; i < a_nSubdivisions; i++) {
		vector3 pointB(a_fOuterRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (-a_fHeight)*0.5, a_fOuterRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));
		bottomOuterPoints.push_back(pointB);

		vector3 pointA(a_fOuterRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (a_fHeight)*0.5, a_fOuterRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));
		topOuterPoints.push_back(pointA);

		vector3 pointC(a_fInnerRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (-a_fHeight)*0.5, a_fInnerRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));
		bottomInnerPoints.push_back(pointC);

		vector3 pointD(a_fInnerRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)), (a_fHeight)*0.5, a_fInnerRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)));
		topInnerPoints.push_back(pointD);
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		if (i == a_nSubdivisions - 1) {
			AddQuad(topInnerPoints[0], topInnerPoints[i], topOuterPoints[0], topOuterPoints[i]);
			AddQuad(bottomInnerPoints[i], bottomInnerPoints[0], bottomOuterPoints[i], bottomOuterPoints[0]);
			AddQuad(bottomOuterPoints[i], bottomOuterPoints[0], topOuterPoints[i], topOuterPoints[0]);
			AddQuad(bottomInnerPoints[0], bottomInnerPoints[i], topInnerPoints[0], topInnerPoints[i]);
		}
		else {
			AddQuad(topInnerPoints[i + 1], topInnerPoints[i], topOuterPoints[i + 1], topOuterPoints[i]);
			AddQuad(bottomInnerPoints[i], bottomInnerPoints[i + 1], bottomOuterPoints[i], bottomOuterPoints[i + 1]);
			AddQuad(bottomOuterPoints[i], bottomOuterPoints[i + 1], topOuterPoints[i], topOuterPoints[i + 1]);
			AddQuad(bottomInnerPoints[i + 1], bottomInnerPoints[i], topInnerPoints[i + 1], topInnerPoints[i]);
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateTorus(float a_fOuterRadius, float a_fInnerRadius, int a_nSubdivisionsA, int a_nSubdivisionsB, vector3 a_v3Color)
{
	if (a_fOuterRadius < 0.01f)
		a_fOuterRadius = 0.01f;

	if (a_fInnerRadius < 0.005f)
		a_fInnerRadius = 0.005f;

	if (a_fInnerRadius > a_fOuterRadius)
		std::swap(a_fInnerRadius, a_fOuterRadius);

	if (a_nSubdivisionsA < 3)
		a_nSubdivisionsA = 3;
	if (a_nSubdivisionsA > 360)
		a_nSubdivisionsA = 360;

	if (a_nSubdivisionsB < 3)
		a_nSubdivisionsB = 3;
	if (a_nSubdivisionsB > 360)
		a_nSubdivisionsB = 360;

	Release();
	Init();

	// Replace this with your code

	float circleRadius = (a_fOuterRadius - a_fInnerRadius) / 2;

	vector3 centerPoint(0, 0, 0);
	vector3 inner3oclock(a_fInnerRadius, 0, 0);
	vector3 outer3oclock(a_fOuterRadius, 0, 0);
	vector3 crossSection3Center((a_fInnerRadius + a_fOuterRadius) / 2, 0, 0);
	vector3 inner9oclock(-a_fInnerRadius, 0, 0);
	vector3 outer9oclock(-a_fOuterRadius, 0, 0);
	vector3 crossSection9Center(((-a_fInnerRadius) + (-a_fOuterRadius)) / 2, 0, 0);
	vector3 inner12oclock(0, a_fInnerRadius, 0);
	vector3 outer12oclock(0, a_fOuterRadius, 0);
	vector3 crossSection12Center(0, (a_fInnerRadius + a_fOuterRadius) / 2, 0);
	vector3 inner6oclock(0, -a_fInnerRadius, 0);
	vector3 outer6oclock(0, -a_fOuterRadius, 0);
	vector3 crossSection6Center(0, ((-a_fInnerRadius) + (-a_fOuterRadius)) / 2, 0);

	GenerateCircle(crossSection12Center, circleRadius, a_nSubdivisionsA, a_v3Color);
	GenerateCircle(crossSection3Center, circleRadius, a_nSubdivisionsA, a_v3Color);
	GenerateCircle(crossSection6Center, circleRadius, a_nSubdivisionsA, a_v3Color);
	GenerateCircle(crossSection9Center, circleRadius, a_nSubdivisionsB, a_v3Color);
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}
void MyMesh::GenerateSphere(float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	//Sets minimum and maximum of subdivisions
	if (a_nSubdivisions < 1)
	{
		GenerateCube(a_fRadius * 2.0f, a_v3Color);
		return;
	}
	if (a_nSubdivisions > 6)
		a_nSubdivisions = 6;

	Release();
	Init();

	// Replace this with your code
	vector3 topPoint(0, a_fRadius, 0);
	vector3 bottomPoint(0, -a_fRadius, 0);

	std::vector<std::vector<vector3>> circles;

	for (int i = 0; i < a_nSubdivisions; i++) {
		std::vector<vector3> list;
		circles.push_back(list);
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		for (int j = 0; j < a_nSubdivisions; j++) {

			float height = a_fRadius - (((2 * a_fRadius) / a_nSubdivisions)* i);
			float radius = sqrt((a_fRadius*a_fRadius) - (height*height));

			float x = radius*sin((glm::radians(360 / (float)a_nSubdivisions)*j));
			float y = height;
			float z = radius*cos((glm::radians(360 / (float)a_nSubdivisions)*j));
			vector3 point(x, y, z);

			circles[i].push_back(point);
		}
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		if (i == a_nSubdivisions - 1) {
			AddTri(circles[0][i], circles[0][0], topPoint);
			AddTri(circles[a_nSubdivisions - 1][0], circles[a_nSubdivisions - 1][i], bottomPoint);
		}
		else {
			AddTri(circles[0][i + 1], circles[0][i], topPoint);
			AddTri(circles[a_nSubdivisions - 1][i + 1], circles[a_nSubdivisions - 1][i], bottomPoint);
		}
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		for (int j = 0; j < a_nSubdivisions; j++) {
			if (i == a_nSubdivisions - 1) {
				continue;
			}
			if (j == a_nSubdivisions - 1) {
				AddQuad(circles[i + 1][j], circles[i + 1][0], circles[i][j], circles[i][0]);
			}
			else {
				AddQuad(circles[i + 1][j], circles[i + 1][j + 1], circles[i][j], circles[i][j + 1]);
			}
		}
	}
	// -------------------------------

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}

void MyMesh::GenerateCircle(vector3 a_v3Center, float a_fRadius, int a_nSubdivisions, vector3 a_v3Color)
{
	Release();
	Init();

	if (a_fRadius < 0.01f)
		a_fRadius = 0.01f;

	if (a_nSubdivisions < 3)
		a_nSubdivisions = 3;
	if (a_nSubdivisions > 360)
		a_nSubdivisions = 360;

	/*
	Calculate a_nSubdivisions number of points around a center point in a radial manner
	then call the AddTri function to generate a_nSubdivision number of faces
	*/

	std::vector<vector3> pointsList;

	for (int i = 0; i < a_nSubdivisions; i++) {
		vector3 point(a_fRadius*sin((glm::radians(360 / (float)a_nSubdivisions)*i)) + a_v3Center.x, a_fRadius*cos((glm::radians(360 / (float)a_nSubdivisions)*i)) + a_v3Center.y, 0 + a_v3Center.z);

		pointsList.push_back(point);
	}

	for (int i = 0; i < a_nSubdivisions; i++) {
		if (i == a_nSubdivisions - 1) {
			AddTri(pointsList[i], pointsList[0], a_v3Center);
			AddTri(pointsList[0], pointsList[i], a_v3Center);
		}
		else {
			AddTri(pointsList[i], pointsList[i + 1], a_v3Center);
			AddTri(pointsList[i + 1], pointsList[i], a_v3Center);
		}

	}

	// Adding information about color
	CompleteMesh(a_v3Color);
	CompileOpenGL3X();
}