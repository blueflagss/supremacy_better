#include "includes.h"

bool Hooks::InPrediction( ) {
	return g_hooks.m_prediction.GetOldMethod< InPrediction_t >( CPrediction::INPREDICTION )( this );
}

int old_num = 0;

void Hooks::RunCommand( Entity* ent, CUserCmd* cmd, IMoveHelper* movehelper ) {
	if( cmd->m_tick >= std::numeric_limits< int >::max( ) )
		return;

	if ( g_cl.m_cmd->m_command_number > old_num ) {
		old_num = g_cl.m_cmd->m_command_number;
	}

	g_hooks.m_prediction.GetOldMethod< RunCommand_t >( CPrediction::RUNCOMMAND )( this, ent, cmd, movehelper );
	
	if ( ent )
		*( int** )( std::uintptr_t ( ent ) + 0x3238 ) = 0;
}


void Hooks::PreEntityPacketReceived( int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed ) {
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	if ( !g_cl.m_processing || !g_cl.m_local )
		return g_hooks.m_prediction.GetOldMethod< PreEntityPacketReceived_t >( CPrediction::PREENTITYPACKETRECEIVED )( this, commands_acknowledged, current_world_update_packet, server_ticks_elapsed );

	if ( commands_acknowledged > 0 ) {
		g_netdata.pre_update( g_cl.m_local );
	}

	return g_hooks.m_prediction.GetOldMethod< PreEntityPacketReceived_t >( CPrediction::PREENTITYPACKETRECEIVED )( this, commands_acknowledged, current_world_update_packet, server_ticks_elapsed );
}

void Hooks::PostNetworkDataReceived( int commands_acknowledged ) {
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	if ( !g_cl.m_processing || !g_cl.m_local )
		return g_hooks.m_prediction.GetOldMethod< PostNetworkDataReceived_t >( CPrediction::POSTNETWORKDATARECEIVED )( this, commands_acknowledged );

	static auto BuildFlattenedChains = pattern::find( g_csgo.m_client_dll, XOR( "55 8B EC 83 EC 18 57 8B F9 89 7D F4 83 7F 14 00 0F 85 ? ? ? ?" ) ).as< void( __thiscall* )( datamap_t* ) >( );

	const auto map = g_cl.m_local->GetPredDescMap( );

	if ( map ) {
		if ( !g_cl.m_rebuilt_datamap ) {
			static auto velocity_modifier_offset = g_netvars.get( HASH( "DT_CSPlayer" ), HASH( "m_flVelocityModifier" ) );

			typedescription_t type_description;

			type_description.field_type = FIELD_FLOAT;
			type_description.field_name = XOR( "m_flVelocityModifier" );
			type_description.field_offset = velocity_modifier_offset;
			type_description.field_size = 1;
			type_description.flags = 0x100;
			type_description.external_name = "";
			type_description.field_size_in_bytes = sizeof( float );
			type_description.field_tolerance = AssignRangeMultiplier( 8, 1.0f );
			const auto type_array = new typedescription_t[ map->data_num_fields + 1 ];

			memcpy( type_array, map->data_desc, sizeof( typedescription_t ) * map->data_num_fields );

			type_array[ map->data_num_fields ] = type_description;

			map->optimized_datamap = nullptr;
			map->data_desc = type_array;
			map->data_num_fields++;
			map->packed_size = 0;

			BuildFlattenedChains( map );

			g_netdata.init( g_cl.m_local );

			g_cl.m_rebuilt_datamap = true;
		}
	}

	if ( commands_acknowledged > 0 ) {
		g_netdata.post_update( g_cl.m_local );
	}

	return g_hooks.m_prediction.GetOldMethod< PostNetworkDataReceived_t >( CPrediction::POSTNETWORKDATARECEIVED )( this, commands_acknowledged );
}
