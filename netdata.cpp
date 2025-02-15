#include "includes.h"

NetData g_netdata{};;

float AssignRangeMultiplier( int bits, double range ) {
	unsigned long high_value;
	if ( bits == 32 )
		high_value = 0xFFFFFFFE;
	else
		high_value = ( ( 1 << ( unsigned long )bits ) - 1 );

	float high_low_mult = high_value / range;
	if ( fabs( range ) <= EQUAL_EPSILON )  
		high_low_mult = high_value;

	if ( ( unsigned long )( high_low_mult * range ) > high_value ||
		( high_low_mult * range ) > ( double )high_value ) {
		float multipliers[ ] = { 0.9999f, 0.99f, 0.9f, 0.8f, 0.7f };
		int i;
		for ( i = 0; i < ARRAYSIZE( multipliers ); i++ ) {
			high_low_mult = ( float )( high_value / range ) * multipliers[ i ];
			if ( ( unsigned long )( high_low_mult * range ) > high_value || ( high_low_mult * range ) > ( double )high_value ) {
			}
			else
				break;
		}

		if ( i == ARRAYSIZE( multipliers ) )
			return 0;
	}

	return high_low_mult;
}

void NetData::init( Player* player ) {
	if ( initialized )
		return;

	ADD_DATAMAP_VAR( float_vars, player->GetPredDescMap( ), "m_flMaxspeed", 1.f / AssignRangeMultiplier( 12, 2048.0f ) );
	ADD_NETVAR( float_vars, "DT_BasePlayer", "m_vecViewOffset[0]", 1.f / AssignRangeMultiplier( 8, 32.0f - ( -32.0f ) ) );
	ADD_NETVAR( float_vars, "DT_BasePlayer", "m_vecViewOffset[1]", 1.f / AssignRangeMultiplier( 8, 32.0f - ( -32.0f ) ) );
	ADD_NETVAR( float_vars, "DT_BasePlayer", "m_vecViewOffset[2]", 1.f / AssignRangeMultiplier( 10, 128.f ) );
	ADD_NETVAR( vec3d_vars, "DT_BasePlayer", "m_aimPunchAngle", 0.031250f );
	ADD_NETVAR( float_vars, "DT_CSPlayer", "m_flVelocityModifier", ( 1.f / AssignRangeMultiplier( 8.f, 1.f ) ) );

	initialized = true;
}

void NetData::pre_update( Player* player ) {
	written_pre_vars = true;

	auto player_addr = reinterpret_cast< size_t >( player );

	for ( auto& var : vec3d_vars )
		var.m_last_value = *reinterpret_cast< vec3_t* >( player_addr + var.m_offset );

	for ( auto& var : float_vars )
		var.m_last_value = *reinterpret_cast< float* >( player_addr + var.m_offset );
}

void NetData::post_update( Player* player ) {
	if ( !written_pre_vars )
		return;

	auto player_addr = reinterpret_cast< size_t >( player );

	for ( auto& var : vec3d_vars ) {
		vec3_t& value = *reinterpret_cast< vec3_t* >( player_addr + var.m_offset );
		value = GetNew( var.m_last_value, value, var.m_tolerance );
	}

	for ( auto& var : float_vars ) {
		float& value = *reinterpret_cast< float* >( player_addr + var.m_offset );
		value = GetNew( var.m_last_value, value, var.m_tolerance );
	}
}