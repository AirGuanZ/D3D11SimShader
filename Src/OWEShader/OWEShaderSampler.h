/*================================================================
Filename: OWEShaderSampler.h
Date: 2017.11.25
Created by AirGuanZ
================================================================*/
#ifndef __OWESHADER_SAMPLER_H__
#define __OWESHADER_SAMPLER_H__

#include <map>

#include "OWEShaderFatalError.h"
#include "OWEShaderObjectBinding.h"
#include "OWEShaderReleaseCOMObjects.h"
#include "OWEShaderUncopiable.h"

namespace _OWEShaderAux
{
    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerObject;

    template<ShaderStageSelector StageSelector>
    class _ShaderStage;

    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerManager;

    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerObject : public _Uncopiable
    {
    public:
        void SetSampler(ID3D11SamplerState *sampler)
        {
            ReleaseCOMObjects(sampler_);
            sampler_ = sampler;
            if(sampler_)
                sampler_->AddRef();
        }

        void Bind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            _BindShaderSampler<StageSelector>(DC, slot_, sampler_);
        }

        void Unbind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            _BindShaderSampler<StageSelector>(DC, slot_, nullptr);
        }

    private:
        friend class _ShaderSamplerManager<StageSelector>;

        _ShaderSamplerObject(UINT slot, ID3D11SamplerState *sampler = nullptr)
            : slot_(slot), sampler_(sampler)
        {
            if(sampler)
                sampler->AddRef();
        }

        ~_ShaderSamplerObject(void)
        {
            ReleaseCOMObjects(sampler_);
        }

    private:
        UINT slot_;
        ID3D11SamplerState *sampler_;
    };

    template<ShaderStageSelector StageSelector>
    class _ShaderSamplerManager : public _Uncopiable
    {
    public:
        using SSObj = _ShaderSamplerObject<StageSelector>;

        ~_ShaderSamplerManager(void)
        {
            for(auto it : SSs_)
            {
                if(it.second.obj)
                    delete it.second.obj;
            }
        }

        void AddShaderSampler(const std::string &name, UINT slot, ID3D11SamplerState *initSampler = nullptr)
        {
            auto it = SSs_.find(name);
            if(it != SSs_.end())
                throw OWEShaderError("Shader sampler name repeated: " + name);
            SSs_[name] = _SSRec{ slot, new SSobj(slot, initSampler) };
        }

        SSObj *GetShaderSamplerObject(const std::string &name)
        {
            auto it = SSs_.find(name);
            if(it == SSs_.end())
                throw OWEShaderError(("Shader sampler not found: " + name).c_str());

            assert(it->second.obj != nullptr);
            return it->second.obj;
        }

        void Bind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            for(auto it : SSs_)
            {
                assert(it.second.obj);
                it.second.obj->Bind(DC);
            }
        }

        void Unbind(ID3D11DeviceContext *DC)
        {
            assert(DC != nullptr);
            for(auto it : SSs_)
            {
                assert(it.second.obj);
                it.second.obj->Unbind(DC);
            }
        }

    private:
        friend class _ShaderStage<StageSelector>;
        struct _SSRec
        {
            UINT slot;
            SSObj *obj;
        };

        _ShaderSamplerManager(const std::map<std::string, _SSRec> &src)
            : SSs_(src)
        {
            for(auto &it : SSs_)
                it.second.obj = new _ShaderSamplerObject<StageSelector>(it.second.slot);
        }

    private:
        std::map<std::string, _SSRec> SSs_;
    };
}

#endif //__OWESHADER_SAMPLER_H__