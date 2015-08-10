#pragma once

#include "../stdafx.h"

class TextureDynamic;
class Material;

class BGNebula
{
public:
	enum TextureScale
	{
		x025,
		x05,
		x1
	};

public:
	void Initialize();
	void Dispose();
	void Update();
	void Render();
	
	inline void SetScale(TextureScale s) { if (s == scale) return; scale = s; scaleChanged = true; }
	inline void IncScale() { TextureScale s = scale == TextureScale::x025 ? TextureScale::x05 : TextureScale::x1; SetScale(s); }
	inline void DecScale() { TextureScale s = scale == TextureScale::x1 ? TextureScale::x05 : TextureScale::x025; SetScale(s); }
	inline TextureScale GetScale() { return scale; }
	inline int GetGenerationTime(int ind) { return iterationTime[ind]; }

private:
	void setupTexture(TextureScale s, bool copyColor);
	void genStep(int offset);
	void combine();

private://static
	static void genThreadFunc(BGNebula* bgn, int offset);
	static void combineThreadFunc(BGNebula* bgn);

private:
	TextureDynamic* texture;
	Material* material;
	byte* colors, *ra, *ga, *ba;
	int colorsLength;
	int oldw, oldh;
	TextureScale scale;

	std::thread* genThread0;
	std::thread* genThread1;
	std::thread* genThread2;
	std::thread* combineThread;
	bool generate;
	bool threadExit;
	float simplexz;

	bool scaleChanged;
	bool updateResolution[4];
	int iterationTime[4];
	bool recombineArray;
};