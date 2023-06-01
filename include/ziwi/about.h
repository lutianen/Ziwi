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
    QPushButton *exitBtn;
    QLabel *infoLabel;
    QLabel *titleLabel;
    QTextEdit *infoTextEdit;
    void initUiComponent(void);

public slots:
    void exitBtnClicked(void);
};
}  // namespace ziwi
}  // namespace Lux
