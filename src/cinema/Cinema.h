#include <vector>
#include "ospray/include/ospray/ospray.h"

#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

#include "Renderer.h"

using namespace std;
using namespace rapidjson;

class Variable
{
public:
	Variable(string n);

	virtual void ReInitialize(Renderer& r);
	virtual void  Render(Renderer& r, string s, Document& doc) = 0;
	virtual string GatherTemplate(string, Document& doc) = 0;

	void RenderDown(Renderer& r, string s, Document& doc);

	Variable *getDown() {return down;}
	void setDown(Variable *v) {down = v;}

	virtual int count() = 0;

protected:
	Variable *down;
	string name;
};

class CameraVariable : public Variable
{
public:
	CameraVariable(vector<int> p, vector<int> t);

	virtual void  ReInitialize(Renderer& r) {};								// Stop downward pass
	virtual void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	void ResetCount() { knt = 0; }

	int count() {
		return phis.size() * thetas.size();
	}

protected:
	void RenderShot(Renderer&, string, Document&);

	int knt;

	vector<int> phis;
	vector<int> thetas;
};

class VolumeRenderingVariable : public Variable
{
public:
	VolumeRenderingVariable(vector<int>);

	virtual void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	int count() {
		return onOff.size() * down->count();
	}

protected:
	void RenderShot(Renderer&, string, Document&);

	vector<int> onOff;
};

class SlicePlaneVariable : public Variable
{
public:
	SlicePlaneVariable(string n, vector<int> c, vector<int> v, vector<int> f, vector<int> vals);
	SlicePlaneVariable(string n, vector<int> v, vector<int> f, vector<int> vals);

	void  Render(Renderer& r, string s, Document& doc);
	string GatherTemplate(string, Document& doc);

	int count() {
		return clip.size() * visible.size() * flip.size() * values.size() * down->count();
	}

private:
	int axis;
	vector<int> clip;
	vector<int> visible;
	vector<int> flip;
	vector<int> values;
};

class IsosurfaceVariable : public Variable
{
public:
	IsosurfaceVariable(string n, vector<int> v);

	virtual void ReInitialize(Renderer& r);
	void  Render(Renderer& r, string s, Document& doc);
	virtual string GatherTemplate(string, Document& doc);

	int count() {
		return values.size() * down->count();
	}

private:
	vector<int> values;
};

class Cinema 
{
public:
		Cinema(int *argc, const char **argv);
		~Cinema();

		void Render(Renderer&, int t);

		void WriteInfo();

		void AddVariable(Variable *v);

		int Count() { return variableStack ? variableStack->count() : -1 ; }

private:
		Variable *variableStack;
		vector<int> timesteps;
};
