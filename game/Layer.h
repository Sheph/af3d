/*
 * Copyright (c) 2020, Stanislav Vorobiov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _LAYER_H_
#define _LAYER_H_

#include "AObject.h"
#include "af3d/EnumSet.h"

namespace af3d
{
    enum class Layer
    {
        General = 0,
        Player = 1,
        Enemy = 2,
        PlayerMissile = 3,
        EnemyMissile = 4,
        Floor = 5,
        Wall = 6,
        Blocker = 7,
        Ally = 8,
        AllyMissile = 9,
        BigEnemy = 10,
        Prop = 11,
        NeutralMissile = 12,
        Max = NeutralMissile
    };

    using Layers = EnumSet<Layer>;

    extern const Layers layersSolid;

    extern const APropertyTypeEnumImpl<Layer> APropertyType_Layer;
    extern const APropertyTypeArray APropertyType_ArrayLayer;
    extern const APropertyTypeArray APropertyType_ArrayArrayLayer;
}

#endif
