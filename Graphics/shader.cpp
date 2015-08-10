#include "shader.h"

#include "graphicsEngine.h"
#include "shaderManager.h"

Shader::Shader()
{
	devcon = GraphicsEngine::GetContext();
	pLayout = null;
}

void Shader::Dispose()
{
	for (int i = 0; i < cBuffersVertexLength; i++)
		bVertexCBuffers[i]->Release();
	delete[] bVertexCBuffers;
	for (int i = 0; i < cBuffersGeometryLength; i++)
		bGeometryCBuffers[i]->Release();
	delete[] bGeometryCBuffers;
	for (int i = 0; i < cBuffersPixelLength; i++)
		bPixelCBuffers[i]->Release();
	delete[] bPixelCBuffers;

	sTexture->Release();

	if (pLayout)
		pLayout->Release();

	pVS->Release();
	pPS->Release();
	if (pGS)
		pGS->Release();

	blobVS->Release();
	blobPS->Release();
	if (blobGS)
		blobGS->Release();
}

void Shader::Activate()
{
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);
	devcon->GSSetShader(pGS, 0, 0);

	for (int i = 0; i < cBuffersVertexLength; i++)
		devcon->VSSetConstantBuffers(i, 1, &bVertexCBuffers[i]);
	for (int i = 0; i < cBuffersGeometryLength; i++)
		devcon->GSSetConstantBuffers(i + cBuffersVertexLength, 1, &bGeometryCBuffers[i]);
	for (int i = 0; i < cBuffersPixelLength; i++)
		devcon->PSSetConstantBuffers(i + cBuffersVertexLength + cBuffersGeometryLength, 1, &bPixelCBuffers[i]);

	devcon->PSSetSamplers(0, 1, &sTexture);
	devcon->IASetInputLayout(pLayout);
}

void Shader::Deactivate()
{
}

void Shader::SetProjectionMatrix(D3DXMATRIX* projection)
{
	if (ShaderManager::GetActiveShader() != this)
		return;
	devcon->UpdateSubresource(bVertexCBuffers[0], 0, nullptr, projection, 0, 0);
}

void Shader::SetViewMatrix(D3DXMATRIX* view)
{
	if (ShaderManager::GetActiveShader() != this)
		return;
	devcon->UpdateSubresource(bVertexCBuffers[1], 0, nullptr, view, 0, 0);
}

void Shader::SetWorldMatrix(D3DXMATRIX* world)
{
	if (ShaderManager::GetActiveShader() != this)
		return;
	devcon->UpdateSubresource(bVertexCBuffers[2], 0, nullptr, world, 0, 0);
}

void Shader::SetVertexCBuffer(int index, void* value)
{
	if (ShaderManager::GetActiveShader() != this)
		return;
	devcon->UpdateSubresource(bVertexCBuffers[index], 0, nullptr, value, 0, 0);
}

void Shader::SetGeometryCBuffer(int index, void* value)
{
	if (ShaderManager::GetActiveShader() != this)
		return;
	devcon->UpdateSubresource(bGeometryCBuffers[index], 0, nullptr, value, 0, 0);
}

void Shader::SetPixelCBuffer(int index, void* value)
{
	if (ShaderManager::GetActiveShader() != this)
		return;
	devcon->UpdateSubresource(bPixelCBuffers[index], 0, nullptr, value, 0, 0);
}

void Shader::SetTexture(ID3D11ShaderResourceView** texture, int index)
{
	devcon->PSSetShaderResources(index, 1, texture);
}

void Shader::SetInputLayout(const D3D11_INPUT_ELEMENT_DESC* ied3d, int numElements)
{
	if (pLayout)
		pLayout->Release();
	HRESULT res = GraphicsEngine::GetDevice()->CreateInputLayout(ied3d, numElements, blobVS->GetBufferPointer(), blobVS->GetBufferSize(), &pLayout);
	assert(!FAILED(res));
}



Shader* Shader::LoadFromMemory(LPCSTR data)
{
	static HRESULT res;
	static ID3D10Blob *VS, *PS, *GS , *err;

	Shader* s = new Shader();

	res = D3DX11CompileFromMemory(data, strlen(data), 0, 0, 0, "VShader", "vs_5_0", 0, 0, 0, &VS, &err, 0);
	if (FAILED(res))
	{
		char* strerr = (char*)err->GetBufferPointer();
		int asfd = 0;
		goto failed;
	}
	res = D3DX11CompileFromMemory(data, strlen(data), 0, 0, 0, "PShader", "ps_5_0", 0, 0, 0, &PS, &err, 0);
	if (FAILED(res))
	{
		char* strerr = (char*)err->GetBufferPointer();
		int asfd = 0;
		goto failed;
	}
	bool geom = false;
	res = D3DX11CompileFromMemory(data, strlen(data), 0, 0, 0, "GShader", "gs_5_0", 0, 0, 0, &GS, &err, 0);
	if (FAILED(res))
	{
		geom = false;
		char* strerr = (char*)err->GetBufferPointer();
		int asfd = 0;
	}
	else
		geom = true;

	s->blobVS = VS;
	s->blobGS = GS;
	s->blobPS = PS;
	
	res = GraphicsEngine::GetDevice()->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &(s->pVS));
	res = GraphicsEngine::GetDevice()->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &(s->pPS));
	if (geom)
		GraphicsEngine::GetDevice()->CreateGeometryShader(GS->GetBufferPointer(), GS->GetBufferSize(), NULL, &(s->pGS));
	else
		s->pGS = NULL;
	
	static D3D11_INPUT_ELEMENT_DESC ied3d[] =
	{
	    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
	    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};
	res = GraphicsEngine::GetDevice()->CreateInputLayout(ied3d, _countof(ied3d), VS->GetBufferPointer(), VS->GetBufferSize(), &s->pLayout);
	
	D3D11_BUFFER_DESC cBufferDesc;
	cBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	cBufferDesc.ByteWidth = sizeof(D3DXMATRIX);
	cBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cBufferDesc.CPUAccessFlags = 0;
	cBufferDesc.MiscFlags = 0;
	cBufferDesc.StructureByteStride = 0;

	//reflection
	ID3D11ShaderReflection* reflection = null;
	ID3D11ShaderReflectionConstantBuffer* cb;
	D3D11_SHADER_DESC desc;
	D3D11_SHADER_BUFFER_DESC cbdesc;
	ID3D11Buffer* buf;

	//vertex reflection
	res = D3DReflect(VS->GetBufferPointer(), VS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
	reflection->GetDesc(&desc);
	ID3D11Buffer** bufs = new ID3D11Buffer*[desc.ConstantBuffers];
	s->cBuffersVertexSizes = new int[desc.ConstantBuffers];
	for (int i = 0; i < desc.ConstantBuffers; i++)
	{
		cb = reflection->GetConstantBufferByIndex(i);
		cb->GetDesc(&cbdesc);
		cBufferDesc.ByteWidth = cbdesc.Size;
		s->cBuffersVertexSizes[i] = cbdesc.Size;
		buf = NULL;
		res = GraphicsEngine::GetDevice()->CreateBuffer(&cBufferDesc, NULL, &buf);
		bufs[i] = buf;
	}
	s->bVertexCBuffers = bufs;
	s->cBuffersVertexLength = desc.ConstantBuffers;

	//geometry reflection
	if (geom)
	{
		res = D3DReflect(GS->GetBufferPointer(), GS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
		reflection->GetDesc(&desc);
		bufs = new ID3D11Buffer*[desc.ConstantBuffers];
		s->cBuffersGeometrySizes = new int[desc.ConstantBuffers];
		for (int i = 0; i < desc.ConstantBuffers; i++)
		{
			cb = reflection->GetConstantBufferByIndex(i);
			cb->GetDesc(&cbdesc);
			cBufferDesc.ByteWidth = cbdesc.Size;
			s->cBuffersGeometrySizes[i] = cbdesc.Size;
			buf = NULL;
			res = GraphicsEngine::GetDevice()->CreateBuffer(&cBufferDesc, NULL, &buf);
			bufs[i] = buf;
		}
		s->bGeometryCBuffers = bufs;
		s->cBuffersGeometryLength = desc.ConstantBuffers;
	}

	//pixel reflection
	res = D3DReflect(PS->GetBufferPointer(), PS->GetBufferSize(), IID_ID3D11ShaderReflection, (void**)&reflection);
	reflection->GetDesc(&desc);
	bufs = new ID3D11Buffer*[desc.ConstantBuffers];
	s->cBuffersPixelSizes = new int[desc.ConstantBuffers];
	for (int i = 0; i < desc.ConstantBuffers; i++)
	{
		cb = reflection->GetConstantBufferByIndex(i);
		cb->GetDesc(&cbdesc);
		cBufferDesc.ByteWidth = cbdesc.Size;
		s->cBuffersPixelSizes[i] = cbdesc.Size;
		buf = NULL;
		res = GraphicsEngine::GetDevice()->CreateBuffer(&cBufferDesc, NULL, &buf);
		bufs[i] = buf;
	}
	s->bPixelCBuffers = bufs;
	s->cBuffersPixelLength = desc.ConstantBuffers;

	static D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	
	res = GraphicsEngine::GetDevice()->CreateSamplerState(&samplerDesc, &s->sTexture);

	return s;

	failed:
	delete s;
	return NULL;
}