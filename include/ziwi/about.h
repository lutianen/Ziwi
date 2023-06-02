#pragma once

#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

namespace Lux {
namespace ziwi {
class About : public QWidget {
    Q_OBJECT
public:
    explicit About(QWidget *parent = 0);
    ~About();

private:
    QLabel *infoLabel;
    QLabel *titleLabel;
    QTextEdit *infoTextEdit;
    void initUiComponent(void);
};
}  // namespace ziwi
}  // namespace Lux
