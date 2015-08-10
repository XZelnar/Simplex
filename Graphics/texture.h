#pragma once

#include "../stdafx.h"

class Texture
{
public:
	Texture();
	Texture(ID3D11ShaderResourceView* _texture);
	virtual void Dispose();
	ID3D11ShaderResourceView* GetTexture();
	void ApplyTexture(int index = 0);
	const wchar_t* GetFilename() { return filename; }

public://static
	static Texture* LoadFromFile(LPCWSTR filename);

protected:
	ID3D11ShaderResourceView* texture;
	wchar_t* filename;
};