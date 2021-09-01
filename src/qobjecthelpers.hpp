#ifndef QOBJECTHELPERS_HPP
#define QOBJECTHELPERS_HPP

#define RECONNECT(target, signal, owner, slot)                                 \
  QObject::disconnect(target, signal, owner, nullptr);                         \
  QObject::connect(target, signal, owner, slot);

#endif // QOBJECTHELPERS_HPP
