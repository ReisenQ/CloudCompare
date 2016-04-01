#ifndef CC_FRUSTUM_HEADER
#define CC_FRUSTUM_HEADER

//CCLib
#include <CCGeom.h>

//Local
#include "ccGLMatrix.h"

class Plane
{
public:

	Plane()
		: normal(0, 0, 1)
		, constCoef(0)
	{}

	virtual ~Plane()
	{}

	void setCoefficients(float a, float b, float c, float d)
	{
		// set the normal vector
		normal = CCVector3f(a, b, c);
		//compute the lenght of the vector
		float l = normal.norm();
		if (l != 0)
		{
			// normalize the vector
			normal /= l;
			// and divide constCoef as well
			constCoef = d / l;
		}
		else
		{
			constCoef = d;
		}

	}

	float distance(const CCVector3f& p) const
	{
		return normal.dot(p) + constCoef;
	}

public: //members

	CCVector3f normal;
	float constCoef;
};

class AABox
{
public:

	AABox()
		: A(0, 0, 0)
		, B(0, 0, 0)
	{}

	AABox(const CCVector3f& minCorner, const CCVector3f& maxCorner)
		: A(minCorner)
		, B(maxCorner)
	{
		if (A.x > B.x)
			std::swap(A.x, B.x);
		if (A.y > B.y)
			std::swap(A.y, B.y);
		if (A.z > B.z)
			std::swap(A.z, B.z);
	}

	virtual ~AABox()
	{
	}

	CCVector3f getVertexP(const CCVector3f& normal) const
	{
		return CCVector3f
		(
			normal.x > 0 ? B.x : A.x,
			normal.y > 0 ? B.y : A.y,
			normal.z > 0 ? B.z : A.z
		);
	}
	CCVector3f getVertexN(const CCVector3f& normal) const
	{
		return CCVector3f
		(
			normal.x < 0 ? B.x : A.x,
			normal.y < 0 ? B.y : A.y,
			normal.z < 0 ? B.z : A.z
		);
	}

public: //members

	CCVector3f A, B;
};

class AACube
{
public:

	AACube()
		: A(0, 0, 0)
		, d(0)
	{}

	AACube(const CCVector3f& minCorner, float size)
		: A(minCorner)
		, d(size)
	{
	}

	virtual ~AACube()
	{
	}

	CCVector3f getVertexP(const CCVector3f& normal) const
	{
		CCVector3f P = A;

		if (normal.x > 0) P.x += d;
		if (normal.y > 0) P.y += d;
		if (normal.z > 0) P.z += d;
		
		return P;
	}
	
	CCVector3f getVertexN(const CCVector3f& normal) const
	{
		CCVector3f P = A;

		if (normal.x < 0) P.x += d;
		if (normal.y < 0) P.y += d;
		if (normal.z < 0) P.z += d;

		return P;
	}

public: //members

	CCVector3f A;
	float d;
};

class Frustum
{
public:

	Frustum()
	{
	}

	Frustum(const ccGLMatrixd& modelViewMat, const ccGLMatrixd& projMat)
	{
		ccGLMatrixd MP = projMat * modelViewMat;
		initfromMPMatrix(MP);
	}

	virtual ~Frustum()
	{
	}

	enum Intersection
	{
		OUTSIDE = 0,
		INTERSECT = 1,
		INSIDE = 2,
	};

public: //Intersection tests

	Intersection pointInFrustum(const CCVector3f& p) const
	{
		for (int i = 0; i < 6; i++)
		{
			if (pl[i].distance(p) < 0)
			{
				return OUTSIDE;
			}
		}

		return INSIDE;
	}

	Intersection sphereInFrustum(CCVector3f &c, float r) const
	{
		Intersection result = INSIDE;

		for (int i = 0; i < 6; i++)
		{
			float distance = pl[i].distance(c);

			if (distance < -r)
				return OUTSIDE;
			else if (distance < r)
				result = INTERSECT;
		}

		return result;
	}

	Intersection boxInFrustum(const AABox& box) const
	{
		Intersection result = INSIDE;
		for (int i = 0; i < 6; i++)
		{
			if (pl[i].distance(box.getVertexP(pl[i].normal)) < 0)
				return OUTSIDE;
			else if (pl[i].distance(box.getVertexN(pl[i].normal)) < 0)
				result = INTERSECT;
		}

		return result;
	}

	Intersection boxInFrustum(const AACube& cube) const
	{
		Intersection result = INSIDE;
		for (int i = 0; i < 6; i++)
		{
			if (pl[i].distance(cube.getVertexP(pl[i].normal)) < 0)
				return OUTSIDE;
			else if (pl[i].distance(cube.getVertexN(pl[i].normal)) < 0)
				result = INTERSECT;
		}

		return result;
	}

protected: //protected methods

	enum PLANE
	{
		TOP = 0,
		BOTTOM = 1,
		LEFT = 2,
		RIGHT = 3,
		NEARP = 4,
		FARP = 5
	};

	template <typename T> void initfromMPMatrix(const ccGLMatrixTpl<T>& MP)
	{
		const T* m = MP.data();
		pl[NEARP].setCoefficients
			(
			m[3] + m[2],
			m[7] + m[6],
			m[11] + m[10],
			m[15] + m[14]
			);

		pl[FARP].setCoefficients
			(
			m[3] - m[2],
			m[7] - m[6],
			m[11] - m[10],
			m[15] - m[14]
			);

		pl[BOTTOM].setCoefficients
			(
			m[3] + m[1],
			m[7] + m[5],
			m[11] + m[9],
			m[15] + m[13]
			);

		pl[TOP].setCoefficients
			(
			m[3] - m[1],
			m[7] - m[5],
			m[11] - m[9],
			m[15] - m[13]
			);

		pl[LEFT].setCoefficients
			(
			m[3] + m[0],
			m[7] + m[4],
			m[11] + m[8],
			m[15] + m[12]
			);

		pl[RIGHT].setCoefficients
			(
			m[3] - m[0],
			m[7] - m[4],
			m[11] - m[8],
			m[15] - m[12]
			);
	}

protected: //members

	Plane pl[6];
};

#endif // CC_FRUSTUM_HEADER