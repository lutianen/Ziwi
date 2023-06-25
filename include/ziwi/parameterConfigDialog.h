
#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QRadioButton>

namespace Lux {
namespace ziwi {

struct ParaConf {
    // 0 - CE7, 1 - TW2
    unsigned char workspace;
    unsigned int width;
    unsigned int height;
    unsigned int channels;
    unsigned int bpp;
    unsigned int mode;
    bool bigEndian;
};

class ParaConfDialog : public QDialog {
    Q_OBJECT

private:
    QRadioButton *ce7_;
    QRadioButton *tw2_;

    QLineEdit* width_;
    QLineEdit* height_;
    QLineEdit* channels_;

    QRadioButton* radio8_;
    QRadioButton* radio12_;
    QRadioButton* radio16_;

    QRadioButton* radioBigEndian_;
    QRadioButton* radioLittleEndian_;

    QRadioButton* radio18_;
    QRadioButton* radio28_;
    QRadioButton* radio38_;
    QRadioButton* radio48_;
    QRadioButton* radio58_;
    QRadioButton* radioAll8_;

    ParaConf* conf_;

public:
    ParaConfDialog(QDialog* parent = nullptr);
    ParaConf paraConf() const;

public slots:
    void onWidthChanged(const QString& width);
    void onHeightChanged(const QString& height);
    void onChannelsChanged(const QString& channels);
    void onSubmit();

private:
    void onTextChanged();
    // void setUi();
};
}  // namespace ziwi
}  // namespace Lux