#pragma once

#include "../stdafx.h"

class TextureDynamic;
class Material;

class BGStars
{
public:
	void Initialize();
	void Dispose();
	void Update();
	void Render();

	inline int GetGenerationTime() { return iterationTime; }

private:
	void genStars();
	void setupTexture();
	void genStep();

private:
	struct Star
	{
		short x;
		short y;
		byte brightness;
	};

private://static
	static void genThreadFunc(BGStars* bgs);

private:
	TextureDynamic* texture;
	Material* material;
	byte* colors;
	std::vector<Star> stars;
	int starsCount;
	int colorsLength;
	int oldw, oldh;

	std::thread* genThread;
	bool generate;
	bool threadExit;
	float simplexz;

	bool scaleChanged;
	bool updateResolution;
	int iterationTime;
};