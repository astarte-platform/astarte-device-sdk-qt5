/*
 * Copyright (C) 2017 Ispirata Srl
 *
 * This file is part of Astarte.
 * Astarte is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * Astarte is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Astarte.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hyperdriveutils.h"

#include <QtCore/QDateTime>

namespace Hyperdrive {

namespace Utils {

void seedRNG()
{
    qsrand(QDateTime::currentMSecsSinceEpoch());
}

qreal randomizedInterval(qreal minimumInterval, qreal delayCoefficient)
{
    return minimumInterval + (minimumInterval * delayCoefficient * ((qreal)qrand() / RAND_MAX));
}

} // Utils

} // Hyperdrive
