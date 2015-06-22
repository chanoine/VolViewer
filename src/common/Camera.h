#pragma once

#include <ospray/ospray.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include "common.h"

#include <math.h>

#include "Lights.h"

class Camera
{
public:
  Camera() :
				modified(true), 
				phi(0.0),
				theta(0.0)
  {
    frame = osp::affine3f(embree::one);
		ospCamera = ospNewCamera("perspective");
		modified = true;
  }

	~Camera()
	{
		ospRelease(ospCamera);
	}

	void setRenderer(OSPRenderer r);

	osp::vec3f getPos();
	void getPos(float* p);
	void getPos(osp::vec3f &p) ;
	void getPos(float& x, float& y, float& z);

	osp::vec3f getCenter();
	void getCenter(float* d);
	void getCenter(osp::vec3f &d);
	void getCenter(float& x, float& y, float& z);

	osp::vec3f getUp();
	void getUp(float* u);
	void getUp(float& x, float& y, float& z);
	void getUp(osp::vec3f &u);

	void setPhi(int iphi);
	void setTheta(int itheta);

	float			 getFovY();
	float			 getAspect();

	void setFovY(float f);
	void setAspect(float a);

  void setupFrame(osp::vec3f, osp::vec3f, osp::vec3f);

	void saveState(Document &doc, Value &section);
	void loadState(Value& cam);

	void commit();

	void rotateFrame(float t, float p);

	void zoom(float dy);
	Lights *getLights();

private:
	OSPCamera ospCamera;
	OSPRenderer renderer;

	Lights cameraLights;

	float phi, theta; 

  osp::affine3f frame;

  osp::vec3f pos;
  osp::vec3f dir;
  osp::vec3f up;

	float eye_dist;
	osp::vec3f center;

  float fovY;
  float aspect;
  bool modified;
};
