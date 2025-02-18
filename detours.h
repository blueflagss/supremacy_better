#pragma once

namespace detours {
	bool Init( );

	void __fastcall NotePredictionError( void* ecx, void* edx, const vec3_t& delta );

	void __vectorcall CL_Move( float accumulated_extra_samples, bool final_tick );
	void NotifyOnLayerChangeWeight( void* ecx, void* edx, const C_AnimationLayer* layer, const float new_weight );
	void NotifyOnLayerChangeCycle( void* ecx, void* edx, const C_AnimationLayer* layer, const float new_cycle );
	void __fastcall DoProceduralFootPlant( void* ecx, void* edx, int a1, int a2, int a3, int a4 );
	ang_t& __fastcall GetEyeAngles( void* ecx, void* edx );
	void __fastcall UpdateClientSideAnimation( void* ecx, void* edx );
	int __fastcall BaseInterpolatePart1( void* ecx, void* edx, float& curtime, vec3_t& old_origin, ang_t& old_angs, int& no_more_changes );
	void __fastcall ProcessMovement( void* ecx, void* edx, Entity* player, CMoveData* data );
	bool __fastcall SVCMsg_VoiceData( void* ecx, void* edx, void* a2 );
	void __fastcall SetupMovement( void* ecx, void* edx );
	void __fastcall PostNetworkDataReceived( void* ecx, void* edx, int commands_acknowledged );
	void __vectorcall UpdateAnimationState( void* ecx, void* a1, float a2, float a3, float a4, void* a5 );
	bool __fastcall SetupBones( void* ecx, void* edx, BoneArray* out, int max, int mask, float curtime );
	void __fastcall Paint( void* ecx, void* edx, PaintModes_t mode );
	void __fastcall ModifyEyePosition( void* ecx, void* edx, vec3_t& eye_pos );
	int __fastcall PacketStart( void* ecx, void* edx, int incoming_sequence, int outgoing_acknowledged );
	void __fastcall PacketEnd( void* ecx, void* edx );
	bool __fastcall ShouldSkipAnimationFrame( void* ecx, void* edx );
	void __fastcall CheckForSequenceChange( void* ecx, void* edx, void* hdr, int sequence, bool force_new_sequence, bool interpolate );
	void __fastcall StandardBlendingRules( void* ecx, void* edx, void* hdr, void* pos, void* q, float current_time, int bone_mask );

	namespace old {
		inline decltype ( &detours::Paint ) Paint;
		inline decltype ( &detours::CL_Move ) CL_Move;
		inline decltype ( &detours::PacketEnd ) PacketEnd;
		inline decltype ( &detours::GetEyeAngles ) GetEyeAngles;
		inline decltype ( &detours::ShouldSkipAnimationFrame ) ShouldSkipAnimationFrame;
		inline decltype ( &detours::CheckForSequenceChange ) CheckForSequenceChange;
		inline decltype ( &detours::StandardBlendingRules ) StandardBlendingRules;
		inline decltype ( &detours::ProcessMovement ) ProcessMovement;
		inline decltype ( &detours::ModifyEyePosition ) ModifyEyePosition;
		inline decltype ( &detours::UpdateAnimationState ) UpdateAnimationState;
		inline decltype ( &detours::PacketStart ) PacketStart;
		inline decltype ( &detours::BaseInterpolatePart1 ) BaseInterpolatePart1;
		inline decltype ( &detours::SVCMsg_VoiceData ) SVCMsg_VoiceData;
		inline decltype ( &detours::NotifyOnLayerChangeCycle ) NotifyOnLayerChangeCycle;
		inline decltype ( &detours::NotifyOnLayerChangeWeight ) NotifyOnLayerChangeWeight;
		inline decltype ( &detours::SetupMovement ) SetupMovement;
		inline decltype ( &detours::DoProceduralFootPlant ) DoProceduralFootPlant;
		inline decltype ( &detours::SetupBones ) SetupBones;
		inline decltype ( &detours::UpdateClientSideAnimation ) UpdateClientSideAnimation;
		inline decltype ( &detours::NotePredictionError ) NotePredictionError;
	}
}