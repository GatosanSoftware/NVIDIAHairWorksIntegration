﻿#pragma once

struct hwShaderData
{
    hwHShader handle;
    int ref_count;
    ID3D11PixelShader *shader;
    std::string path;

    hwShaderData() : handle(hwNullHandle), ref_count(0), shader(nullptr) {}
    void invalidate() { ref_count = 0; shader = nullptr; path.clear(); }
    operator bool() const { return shader != nullptr; }
};

struct hwAssetData
{
    hwHAsset handle;
    int ref_count;
    hwAssetID aid;
    std::string path;
    hwConversionSettings settings;

    hwAssetData() : handle(hwNullHandle), aid(hwNullAssetID), ref_count(0) {}
    void invalidate() { ref_count = 0; aid = hwNullAssetID; path.clear(); }
    operator bool() const { return aid != hwNullAssetID; }
};

struct hwInstanceData
{
    hwHInstance handle;
    hwInstanceID iid;
    hwHAsset hasset;
    bool cast_shadow;
    bool receive_shadow;

    hwInstanceData() : handle(hwNullHandle), iid(hwNullInstanceID), hasset(hwNullHandle), cast_shadow(false), receive_shadow(false) {}
    void invalidate() { iid = hwNullInstanceID; hasset = hwNullAssetID; cast_shadow = false; receive_shadow = false; }
    operator bool() const { return iid != hwNullInstanceID; }
};

enum hwELightType
{
	hwELightType_Spot,
    hwELightType_Directional,
    hwELightType_Point,
};

struct hwLightData
{
    hwELightType type; int pad[3];
    hwFloat4 position; // w: range
    hwFloat4 direction;
    hwFloat4 color;
	int angle; int pad2[3]; //spot angle

    hwLightData()
        : type(hwELightType_Directional)
        , position({ 0.0f, 0.0f, 0.0f, 0.0f })
        , direction({ 0.0f, 0.0f, 0.0f, 0.0f })
        , color({ 1.0f, 1.0f, 1.0f, 1.0 })
		, angle(180)
    {}
};

struct hwConstantBuffer
{
	//Spherical Harmonics
	hwFloat4 shAr;
	hwFloat4 shAg;
	hwFloat4 shAb;
	hwFloat4 shBr;
	hwFloat4 shBg;
	hwFloat4 shBb;
	hwFloat4 shC;

	//GI Parameters
	hwFloat4 gi_params;

	
    int num_lights; int pad0[3];
    hwLightData lights[hwMaxLights];
    GFSDK_HairShaderConstantBuffer hw;

    hwConstantBuffer() : num_lights(0) {}
};

struct hwShadowParamBuffer
{
	gfsdk_float4x4 worldToShadow[4];
	hwFloat4 splitSpheres[4];
	hwFloat4 shadowSplitSqRadii;
	hwFloat4 LightSplitsNear;
	hwFloat4 LightSplitsFar;
};



class hwContext
{
public:
    static hwSDK* loadSDK();
    static void   unloadSDK();


public:
    hwContext();
    ~hwContext();
    bool valid() const;

    bool initialize(hwDevice *d3d_device);
    void finalize();
    void move(hwContext &from);

    hwHShader       shaderLoadFromFile(const std::string &path);
    void            shaderRelease(hwHShader hs);
    void            shaderReload(hwHShader hs);

    hwHAsset        assetLoadFromFile(const std::string &path, const hwConversionSettings *conv);
    void            assetRelease(hwHAsset ha);
    void            assetReload(hwHAsset ha);
    int             assetGetNumBones(hwHAsset ha) const;
    const char*     assetGetBoneName(hwHAsset ha, int nth) const;
    void            assetGetBoneIndices(hwHAsset ha, hwFloat4 &o_indices) const;
    void            assetGetBoneWeights(hwHAsset ha, hwFloat4 &o_weight) const;
    void            assetGetBindPose(hwHAsset ha, int nth, hwMatrix &o_mat);
    void            assetGetDefaultDescriptor(hwHAsset ha, hwHairDescriptor &o_desc) const;

    hwHInstance     instanceCreate(hwHAsset ha);
    void            instanceRelease(hwHInstance hi);
    void            instanceGetBounds(hwHInstance hi, hwFloat3 &o_min, hwFloat3 &o_max) const;
    void            instanceGetDescriptor(hwHInstance hi, hwHairDescriptor &desc) const;
    void            instanceSetDescriptor(hwHInstance hi, const hwHairDescriptor &desc);
    void            instanceSetTexture(hwHInstance hi, hwTextureType type, hwTexture *tex);
    void            instanceUpdateSkinningMatrices(hwHInstance hi, int num_bones, hwMatrix *matrices);
    void            instanceUpdateSkinningDQs(hwHInstance hi, int num_bones, hwDQuaternion *dqs);

    void beginScene();
    void endScene();
	void initializeDepthStencil(BOOL flipComparison);
    void setViewProjection(const hwMatrix &view, const hwMatrix &proj, float fov);
    void setRenderTarget(hwTexture *framebuffer, hwTexture *depthbuffer);
    void setShader(hwHShader hs);
    void setLights(int num_lights, const hwLightData *lights);
	void setShadowTexture(ID3D11Resource *shadowTexture);
	void setShadowParams(void* shadowCB);
	void setSphericalHarmonics(const hwFloat4 &Ar, const hwFloat4 &Ag, const hwFloat4 &Ab, const hwFloat4 &Br, const hwFloat4 &Bg, const hwFloat4 &Bb, const hwFloat4 &C);
	void setGIParameters(const hwFloat4 &Params);
	void setReflectionProbe(ID3D11Resource *tex1, ID3D11Resource *tex2);
    void render(hwHInstance hi);
    void renderShadow(hwHInstance hi);
    void stepSimulation(float dt);
    void flush();

private:
    hwShaderData&   newShaderData();
    hwAssetData&    newAssetData();
    hwInstanceData& newInstanceData();

    typedef std::function<void()> DeferredCall;
    void pushDeferredCall(const DeferredCall &c);
    void setViewProjectionImpl(const hwMatrix &view, const hwMatrix &proj, float fov);
    void setRenderTargetImpl(hwTexture *framebuffer, hwTexture *depthbuffer);
    void setShaderImpl(hwHShader hs);
    void setLightsImpl(int num_lights, const hwLightData *lights);
	void setSphericalHarmonicsImpl(const hwFloat4 &Ar, const hwFloat4 &Ag, const hwFloat4 &Ab, const hwFloat4 &Br, const hwFloat4 &Bg, const hwFloat4 &Bb, const hwFloat4 &C);
	void setGIParametersImpl(const hwFloat4 &Params);
	void setReflectionProbeImpl(ID3D11Resource *tex1, ID3D11Resource *tex2);
    void renderImpl(hwHInstance hi);
    void renderShadowImpl(hwHInstance hi);
    void stepSimulationImpl(float dt);
    hwSRV* getSRV(hwTexture *tex);
    hwRTV* getRTV(hwTexture *tex);

private:
    typedef std::vector<hwShaderData>       ShaderCont;
    typedef std::vector<hwAssetData>        AssetCont;
    typedef std::vector<hwInstanceData>     InstanceCont;
    typedef std::map<hwTexture*, hwSRV*>    SRVTable;
    typedef std::map<hwTexture*, hwRTV*>    RTVTable;
    typedef std::vector<DeferredCall>       DeferredCalls;

    std::mutex              m_mutex;

    ID3D11Device            *m_d3ddev = nullptr;
    ID3D11DeviceContext     *m_d3dctx = nullptr;
    ShaderCont              m_shaders;
    AssetCont               m_assets;
    InstanceCont            m_instances;
    SRVTable                m_srvtable;
    RTVTable                m_rtvtable;
    DeferredCalls           m_commands;
    DeferredCalls           m_commands_back;

    ID3D11DepthStencilState *m_rs_enable_depth;
    ID3D11Buffer            *m_rs_constant_buffer;

    hwConstantBuffer        m_cb;

	ID3D11Resource *reflectionTexture1 = nullptr;
	ID3D11Resource *reflectionTexture2 = nullptr;

	
	ID3D11ShaderResourceView* reflectionSRV1 = nullptr;
	ID3D11ShaderResourceView* reflectionSRV2 = nullptr;

	ID3D11Texture2D *shadowTexture = nullptr;
	ID3D11ShaderResourceView *shadowSRV =  nullptr;

	ID3D11Buffer *shadowBuffer = nullptr;
	ID3D11ShaderResourceView* bufferSRV = nullptr;
};
