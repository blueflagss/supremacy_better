#include "includes.h"

void Hooks::LevelInitPreEntity( const char* map ) {
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	g_csgo.cl_updaterate->SetValue( rate );
	g_csgo.cl_cmdrate->SetValue( rate );

	g_aimbot.post_update( );
	g_visuals.m_hit_start = g_visuals.m_hit_end = g_visuals.m_hit_duration = 0.f;

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >( CHLClient::LEVELINITPREENTITY )( this, map );
}

void Hooks::LevelInitPostEntity( ) {
	g_cl.OnMapload( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >( CHLClient::LEVELINITPOSTENTITY )( this );
}

void Hooks::LevelShutdown( ) {
	g_aimbot.post_update( );

	g_cl.m_local = nullptr;
	g_cl.m_weapon = nullptr;
	g_cl.m_processing = false;
	g_cl.m_weapon_info = nullptr;
	g_cl.m_round_end = false;

	g_cl.m_sequences.clear( );

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >( CHLClient::LEVELSHUTDOWN )( this );
}

/*int Hooks::IN_KeyEvent( int evt, int key, const char* bind ) {
	// see if this key event was fired for the drop bind.
	/*if( bind && FNV1a::get( bind ) == HASH( "drop" ) ) {
		// down.
		if( evt ) {
			g_cl.m_drop = true;
			g_cl.m_drop_query = 2;
			g_cl.print( "drop\n" );
		}

		// up.
		else
			g_cl.m_drop = false;

		// ignore the event.
		return 0;
	}

	return g_hooks.m_client.GetOldMethod< IN_KeyEvent_t >( CHLClient::INKEYEVENT )( this, evt, key, bind );
}*/

void Hooks::FrameStageNotify( Stage_t stage ) {
	// save stage.
	if ( stage != FRAME_START )
		g_cl.m_stage = stage;

	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >( g_csgo.m_engine->GetLocalPlayer( ) );

	if ( stage == FRAME_RENDER_START ) {
		// apply local player animated angles.
		if ( g_csgo.m_input->CAM_IsThirdPerson ( ) )
			g_csgo.m_prediction->SetLocalViewAngles ( g_cl.m_radar );

		//if ( g_cl.m_local && g_cl.m_local->alive ( ) ) {
		//	g_cl.m_local->m_angRotation ( ) = g_cl.m_rotation;
		//	g_cl.m_local->m_angNetworkAngles ( ) = g_cl.m_rotation;
		//}

		// apply local player animation fix.
		g_cl.UpdateAnimations( );
		//g_shots.OnPreFrameStage( );

		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
			if ( !player || !player->alive( ) || player->dormant( ) || player->m_bIsLocalPlayer( ) )
				continue;

			AimPlayer* data = &g_aimbot.m_players[ i - 1 ];

			if ( data->m_records.empty( ) )
				continue;

			auto& front = data->m_records.front( );

			if ( !front )
				continue;

			g_cl.m_usable_bones[ player->index( ) ] = front->m_bones;
			g_cl.m_usable_origin[ player->index( ) ] = front->m_origin;

			player->SetPoseParameters( front->m_poses );
			player->SetAbsAngles( front->m_abs_ang );
		}
		// draw our custom beams.
		g_visuals.DrawBeams( );
	}

	if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END && ( g_cl.m_local && g_cl.m_local->alive( ) ) ) {
		//g_inputpred.stored.m_old_velocity_modifier = g_cl.m_local->m_flVelocityModifier ( );

		//if ( g_cl.m_local->m_flVelocityModifier ( ) < g_inputpred.stored.m_old_velocity_modifier ) {
		//	g_inputpred.stored.m_velocity_modifier = g_inputpred.stored.m_old_velocity_modifier;
		//	g_inputpred.ForceUpdate ( true );
		//}
	}

	// call og.
	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >( CHLClient::FRAMESTAGENOTIFY )( this, stage );

	if ( stage == FRAME_RENDER_START ) {
		// ...
	}

	else if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START ) {
		// restore non-compressed netvars.
		// g_netdata.apply( );

		g_skins.think( );
	}

	else if ( stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END ) {
		g_visuals.NoSmoke( );
	}

	else if ( stage == FRAME_NET_UPDATE_END ) {
		// update all players.
		for ( int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i ) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >( i );
			if ( !player || player->m_bIsLocalPlayer( ) )
				continue;

			AimPlayer* data = &g_aimbot.m_players[ i - 1 ];
			data->OnNetUpdate( player );
		}
	}
}