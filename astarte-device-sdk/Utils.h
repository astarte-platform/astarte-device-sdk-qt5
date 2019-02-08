#ifndef UTILS_H
#define UTILS_H

#include <QtCore/QHash>

namespace Utils {

bool verifyPathMatch(const QByteArrayList path1, const QByteArrayList path2);
bool verifySplitTokenMatch(const QHash<QByteArray, QByteArrayList> &token2, const QHash<QByteArray, QByteArrayList> &token1);

} // Utils

#endif // UTILS_H
