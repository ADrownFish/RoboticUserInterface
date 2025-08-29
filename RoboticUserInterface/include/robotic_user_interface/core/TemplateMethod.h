#pragma once

#include <QString>
#include <QList>
#include <QJsonArray>
#include <QJsonObject>

template<typename T>
void InsertHistory(QList<T> &list, const T& value, int count = 5) {
  list.removeAll(value);
  list.prepend(value);
  while (list.size() > count) {
    list.removeLast();
  }
}

template <typename T>
QJsonArray toJsonArray(const QList<T>&vec) {
  QJsonArray array;
  for (const auto& item : vec) {
    array.append(item);
  }
  return array;
}

template <typename T>
QList<T> toList(const QJsonArray& array) {
  QList<T> list;
  for (const auto& value : array) {
    list.append(static_cast<T>(value.toVariant().value<T>()));
  }
  return list;
}