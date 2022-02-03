#include "ParticleEmitter.h"

#include "FrameResource.h"

//template <class Particle, class Emission, class Update, class Deletion>
//void ParticleEmitter<Particle, Emission, Update, Deletion>::Update(float deltaTime)
//{
//	Emit();
//	UpdatePositions(deltaTime);
//	DeleteParticles();
//}

//template <class Particle, class Emission, class Update, class Deletion>
//void ParticleEmitter<Particle, Emission, Update, Deletion>::DrawParticles(ID3D12GraphicsCommandList* cmdList,
//    ID3D12Resource* objCBResource, ID3D12Resource* matCBResource)
//{
//    UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
//    UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ToonMaterialConstants));
//
//    
//
//    // For each render item...
//    for (size_t i = 0; i < m_vParticles.size(); ++i)
//    {
//        Particle p = m_vParticles[i];
//        if (p.m_alive)
//        {
//
//            cmdList->IASetVertexBuffers(0, 1, &p.m_renderItem->Geo->VertexBufferView());
//            cmdList->IASetIndexBuffer(&p.m_renderItem->Geo->IndexBufferView());
//            cmdList->IASetPrimitiveTopology(p.m_renderItem->PrimitiveType);
//
//            D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objCBResource->GetGPUVirtualAddress() + p.m_renderItem->ObjCBIndex * objCBByteSize;
//            D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCBResource->GetGPUVirtualAddress() + p.m_renderItem->Mat->MatCBIndex * matCBByteSize;
//
//            cmdList->SetGraphicsRootConstantBufferView(0, objCBAddress);
//            cmdList->SetGraphicsRootConstantBufferView(1, matCBAddress);
//
//            cmdList->DrawIndexedInstanced(p.m_renderItem->IndexCount, 1, p.m_renderItem->StartIndexLocation, p.m_renderItem->BaseVertexLocation, 0);
//
//        }
//    }
//}


void Emission_policies::SphereEmission::Emit(float deltaTime)
{
	
}

void Update_policies::Constant::UpdatePositions(float deltaTime)
{
	
}

void Deletion_policies::LifeSpan::DeleteParticles()
{
	
}
