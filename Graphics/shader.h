#pragma once

#include "../stdafx.h"

class Texture;
class Transformation;

class Shader
{
	friend class Material;
	friend class ShaderManager;

public:
	Shader();
	void Dispose();
	void Activate();
	void Deactivate();

private:
	void SetVertexCBuffer(int index, void* value);
	void SetGeometryCBuffer(int index, void* value);
	void SetPixelCBuffer(int index, void* value);

	void SetProjectionMatrix(D3DXMATRIX* projection);
	void SetViewMatrix(D3DXMATRIX* view);
	void SetWorldMatrix(D3DXMATRIX* world);
	void SetTexture(ID3D11ShaderResourceView** texture, int index = 0);
	void SetInputLayout(const D3D11_INPUT_ELEMENT_DESC* ied3d, int numElements);

protected:
	ID3D10Blob* blobVS, *blobGS, *blobPS;
	ID3D11VertexShader* pVS;
	ID3D11PixelShader* pPS;
	ID3D11GeometryShader* pGS;
	ID3D11InputLayout* pLayout;
	ID3D11DeviceContext* devcon;
	//ID3D11Buffer* bMatrixView;
	//ID3D11Buffer* bMatrixProjection;
	//ID3D11Buffer* bMatrixWorld;
	ID3D11Buffer** bVertexCBuffers;
	ID3D11Buffer** bGeometryCBuffers;
	ID3D11Buffer** bPixelCBuffers;
	int* cBuffersVertexSizes;
	int* cBuffersGeometrySizes;
	int* cBuffersPixelSizes;
	int  cBuffersVertexLength;
	int  cBuffersGeometryLength;
	int  cBuffersPixelLength;
	ID3D11SamplerState* sTexture;

public://static
	static Shader* LoadFromMemory(LPCSTR filename);
};