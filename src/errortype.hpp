#ifndef ERRORTYPE_HPP
#define ERRORTYPE_HPP

#include <QMetaType>

enum class ErrorType { GENERIC = 0, STDERR, ALREADY_RUNNING };
Q_DECLARE_METATYPE(ErrorType);

#endif // ERRORTYPE_HPP
