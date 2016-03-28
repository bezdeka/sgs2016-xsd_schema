#ifndef XSD_H
#define XSD_H

#include <QMainWindow>

namespace Ui {
class xsd;
}

class xsd : public QMainWindow
{
    Q_OBJECT

public:
    explicit xsd(QWidget *parent = 0);
    ~xsd();

private slots:
    void on_actionExit_triggered();

    void on_action_Read_XSD_schema_triggered();

    void on_action_Export_XSD_schema_triggered();

    void XSLT_run(QString stylesheet);

    void XSLT_element_remove(QString &stylesheet, int row, int col);

private:
    Ui::xsd *ui;
};

#endif // XSD_H
