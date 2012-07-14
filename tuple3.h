#pragma once
#include <math.h>
#include <iostream>
#include <assert.h>

class tuple3f
{
public:
	float x, y, z;
	tuple3f(void);
	tuple3f(float x_, float y_, float z_){
		x = x_;
		y=y_;
		z=z_;
	}
	~tuple3f(void);

	void set(float x_, float y_, float z_){
		x = x_;
		y=y_;
		z=z_;
	}
	void set(tuple3f & other)
	{
		x= other.x;
		y= other.y;
		z=other.z;
	}

	void normalize(void){
		float temp = x*x + y*y + z*z;
		temp = pow(temp, 0.5f);
		x = x/temp;
		y= y/temp;
		z= z/temp;
	}

	static tuple3f cross(tuple3f &a, tuple3f &b){
		float x = a.y*b.z - a.z*b.y;
		float y = a.z*b.x - a.x*b.z;
		float z = a.x*b.y - a.y*b.x;
		return tuple3f(x,y,z);
	}

	tuple3f cross(tuple3f &b){
		float x = this->y*b.z - this->z*b.y;
		float y = this->z*b.x - this->x*b.z;
		float z = this->x*b.y - this->y*b.x;
		return tuple3f(x,y,z);
	}

	static float crossNorm(tuple3f &a, tuple3f &b){
		float x = a.y*b.z - a.z*b.y;
		float y = a.z*b.x - a.x*b.z;
		float z = a.x*b.y - a.y*b.x;
		return pow(x*x + y*y + z*z, 0.5f);
	}

	static float cot(tuple3f &a, tuple3f &b){
		float t = crossNorm(a,b);
		float temp = a.dot(b) /crossNorm(a,b);
		if(temp < -10E15 || temp > 10E15 || temp*0!=0){
			std::cout << "***warning*** : numerical stability in tuple3f::cot val = "<<temp <<"\n";
		}
		return temp;
		//return a.dot(b)/(t>0.0001f? t: 0.0001f);
	}

		//cot at b of the triangle a, b,c
	static float cotPoints(tuple3f &a, tuple3f &b, tuple3f &c){
		//cot at b of the triangle a, b,c
		float a_bDotc_b =(a.x-b.x)*(c.x - b.x) + (a.y-b.y)*(c.y - b.y) + (a.z-b.z)*(c.z - b.z);
		float x = (a.y-b.y)*(c.z - b.z) - (a.z-b.z)*(c.y - b.y);
		float y = (a.z-b.z)*(c.x - b.x) - (a.x-b.x)*(c.z - b.z);
		float z = (a.x-b.x)*(c.y - b.y) - (a.y-b.y)*(c.x - b.x);
		float norm_sqr_a_bXc_b = x*x + y*y + z*z;
		return a_bDotc_b/pow(norm_sqr_a_bXc_b, 0.5f);
	}	

	float norm(void){
		float temp = x*x + y*y + z*z;
		temp = pow(temp, 0.5f);
		return temp;
	}

	float normSqr(void){
		return x*x + y*y + z*z;
	}
	
	float dot(tuple3f &other){
		return x*other.x + y * other.y + z * other.z;
	}

	//(a-b) dot(c-b)
	static float a_bDotc_b( tuple3f &a, tuple3f &b , tuple3f & c )
	{
		return (a.x - b.x)*(c.x-b.x) + (a.y - b.y)*(c.y-b.y) + (a.z - b.z)*(c.z-b.z);
	}

	static float sinPoints(tuple3f &a, tuple3f &b, tuple3f &c){
		float t = crossNorm(a-b,c-b);
		t/= (a-b).norm() * (c-b).norm();
		return t;
	}

	static float cosPoints( tuple3f& a, tuple3f& b, tuple3f& c ){
		/*float temp = a_bDotc_b(a,b,c) / (b-a).norm() /(c-b).norm();
		if(temp > 1 || temp < - 1){
			std::cout << "dammnit";
			throw std::exception("Assertion failed in tuple3h::cosPoints");
		}*/
		return a_bDotc_b(a,b,c) / (b-a).norm() /(c-b).norm();
	}

	//////////////////////////////////////////////////////////////////////////
	//returns a
	static float angle(tuple3f& a, tuple3f& b,tuple3f& c){
		return acos(cosPoints(a,b,c));
	}


	tuple3f operator + (tuple3f &other){
		return tuple3f(x + other.x, y+ other.y, z + other.z);
	}

	void operator += (tuple3f &other){
		x += other.x;
		y += other.y;
		z += other.z;
	}

	void operator -= (tuple3f &other){
		x -= other.x;
		y -= other.y;
		z -= other.z;
	}

	void operator *= (float other){
		x *= other;
		y *= other;
		z *= other;
	}

	tuple3f operator * (float other){
		return tuple3f(x * other, y * other, z * other);
	}

	
	tuple3f operator - (tuple3f &other){
		return tuple3f(x - other.x, y- other.y, z - other.z);
	}

	tuple3f operator - (){
		return tuple3f(-x , -y, -z );
	}

	tuple3f operator - (float &other){
		return tuple3f(x - other, y- other, z - other);
	}

	void operator -= (float &other){
		x -= other;
		y -= other;
		z -= other;
	}

};

// for fun, could use pair..
class tuple2i
{
public:
	int a, b;
	tuple2i(void);
	tuple2i(int x_, int y_){
		a = x_;
		b=y_;
	}
	~tuple2i(void){}

	void set(int x_, int y_){
		a = x_;
		b=y_;
	}

	//////////////////////////////////////////////////////////////////////////
	//Returns +-1 or 0 if vertex is the start vertex -1, end +1 neither: 0
	//////////////////////////////////////////////////////////////////////////
	int orientation(int vertex){
		if(a==vertex){
			return -1;
		}
		if(b==vertex){
			return 1;
		}
		assert(false);
		return 0;
	}

	bool operator < (const tuple2i & other) const{
		return this->a <other.a || (this->a == other.a && this->b < other.b);
	}
};

class tuple3i
{
public:
	int a, b, c;
	tuple3i(void);
	tuple3i(int x_, int y_, int z_){
		a = x_;
		b=y_;
		c=z_;
	}
	~tuple3i(void);

	void set(int x_, int y_, int z_){
		a = x_;
		b=y_;
		c=z_;
	}

	bool contains(tuple2i & tuple){
		return 	(tuple.a == a || tuple.a == b || tuple.a == c)&&
			(tuple.b == a || tuple.b == b || tuple.b == c);
	}

	bool contains(int what)
	{
		return 	(a == what || what == b || what == c);
	}

	int orientation(tuple2i & edge){
		
		if(a == edge.a && b == edge.b){
			return 1;
		}
		if(b == edge.a && c == edge.b){
			return 1;
		}
		if(c == edge.a && a == edge.b){
			return 1;
		}
		if(contains(edge))
			return -1;

		return 0;
	}
};


