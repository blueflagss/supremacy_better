#include "includes.h"

vec3_t matrix3x4_t::operator*( const vec3_t& vec ) const {
    auto& m = m_flMatVal;

    vec3_t ret;
    ret.x = m[ 0 ][ 0 ] * vec.x + m[ 0 ][ 1 ] * vec.y + m[ 0 ][ 2 ] * vec.z + m[ 0 ][ 3 ];
    ret.y = m[ 1 ][ 0 ] * vec.x + m[ 1 ][ 1 ] * vec.y + m[ 1 ][ 2 ] * vec.z + m[ 1 ][ 3 ];
    ret.z = m[ 2 ][ 0 ] * vec.x + m[ 2 ][ 1 ] * vec.y + m[ 2 ][ 2 ] * vec.z + m[ 2 ][ 3 ];

    return ret;
}

matrix3x4_t matrix3x4_t::operator*( const matrix3x4_t& vm ) {
    matrix3x4_t out;

    math::ConcatTransforms( *this, vm, out );

    return out;
}
