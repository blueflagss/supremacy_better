#include "includes.h"

void Player::ModifyEyePosition( CCSGOPlayerAnimState* state, vec3_t* pos ) {
	if ( !state ) {
		return;
	}

	//  if ( *(this + 0x50) && (*(this + 0x100) || *(this + 0x94) != 0.0 || !sub_102C9480(*(this + 0x50))) )
	if ( state->m_pPlayer &&
		( state->m_bLanding || state->m_pPlayer->m_flDuckAmount( ) != 0.f || !state->m_pPlayer->GetGroundEntity( ) ) ) {
		auto v5 = 8;
		const auto bones = this == g_cl.m_local ? g_cl.m_usable_bones[ this->index( ) ].data( ) : state->m_pPlayer->m_pBoneCache( );

		if ( v5 != -1 && bones ) {
			vec3_t head_pos(
				bones[ 8 ][ 0 ][ 3 ],
				bones[ 8 ][ 1 ][ 3 ],
				bones[ 8 ][ 2 ][ 3 ] );

			auto v12 = head_pos;
			auto v7 = v12.z + 1.7;

			auto v8 = pos->z;
			if ( v8 > v7 ) // if (v8 > (v12 + 1.7))
			{
				float v13 = 0.f;
				float v3 = ( *pos ).z - v7;

				float v4 = ( v3 - 4.f ) * 0.16666667;
				if ( v4 >= 0.f )
					v13 = std::fminf( v4, 1.f );

				( *pos ).z = ( ( ( v7 - ( *pos ).z ) ) * ( ( ( v13 * v13 ) * 3.0 ) - ( ( ( v13 * v13 ) * 2.0 ) * v13 ) ) ) + ( *pos ).z;
			}
		}
	}
}