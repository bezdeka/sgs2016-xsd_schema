#include "xsd.h"
#include "ui_xsd.h"

#include <QXmlStreamReader> // cteni xml do tableWidget
#include <QXmlQuery>        // podpora xslt, potreba pridat QT += xmlpatterns
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QAbstractItemModel>
#include <QFont>
#include <QDebug>

xsd::xsd(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::xsd)
{
    ui->setupUi(this);

    // nastaveni klavesovych zkratek
    ui->actionExit->setShortcut(Qt::Key_Q);
    ui->action_Read_XSD_schema->setShortcut(Qt::Key_R);
    ui->action_Export_XSD_schema->setShortcut(Qt::Key_E);

}

xsd::~xsd()
{
    delete ui;
}

void xsd::on_actionExit_triggered()
{
    close();
}

void xsd::on_action_Read_XSD_schema_triggered()
{
    // pro urychleni testovani nastaveno naprimo
    QFile file(":/files/examples/my1example.xsd");
    //QFile file(":/files/examples/my2example.xsd");
    file.open(QIODevice::ReadOnly);

    QFont font_bold("Ubuntu", 11, QFont::Bold);
    ui->tableWidget->setShowGrid(false);
    ui->tableWidget->horizontalHeader()->hide();
    ui->tableWidget->setRowCount(1);
    ui->tableWidget->setColumnCount(1);

    // cteni XSD schematu
    QXmlStreamReader xml(file.readAll());

    int lvl = 2;
    int max_lvl = 0;
    int row = 0;
    QString val = "";

    qDebug() << "Read_XSD_schema -> element (row-level-max.level)";

    while (!xml.atEnd())
    {

        QCheckBox *checkbox = new QCheckBox();
        checkbox->setChecked(true);

        ui->tableWidget->setRowCount(row+1);
        ui->tableWidget->setCellWidget(row,0,checkbox);

        xml.readNext();

        if(xml.tokenType() == QXmlStreamReader::StartDocument)
        {
            // moznost ziskat version a encoding
        }

        if(xml.tokenType() == QXmlStreamReader::StartElement)
        {
            // podle potreby pridavam sloupce do tableWidget
            // vzdy o 1 vic pro pripadny "attributes value"
            if(max_lvl<lvl){
                max_lvl = lvl;
                ui->tableWidget->setColumnCount(max_lvl+2);
            }

            qDebug() << row << " - " << lvl << " - " << max_lvl;

            if(xml.prefix() == ""){
                val = xml.name().toString();
            }else{
                val = xml.prefix().toString() + ":" + xml.name().toString();
            }

            ui->tableWidget->setItem(row,1,new QTableWidgetItem("element"));
            ui->tableWidget->setItem(row,lvl,new QTableWidgetItem(val));
            ui->tableWidget->item(row,lvl)->setFont(font_bold);

            row++;

            QXmlStreamAttributes attr = xml.attributes();
            for(QXmlStreamAttributes::Iterator it=attr.begin(); it < attr.end(); it++){

                QCheckBox *checkbox = new QCheckBox();
                checkbox->setChecked(true);

                ui->tableWidget->setRowCount(row+1);
                ui->tableWidget->setCellWidget(row,0,checkbox);

                ui->tableWidget->setItem(row,1,new QTableWidgetItem("attributes"));
                ui->tableWidget->setItem(row,lvl,new QTableWidgetItem(it->name().toString()));
                ui->tableWidget->setItem(row,lvl+1,new QTableWidgetItem(it->value().toString()));

                row++;
            }
            lvl += 1;
        }

        if(xml.tokenType() == QXmlStreamReader::Characters)
        {
        }

        if(xml.tokenType() == QXmlStreamReader::Comment)
        {
        }

        if(xml.tokenType() == QXmlStreamReader::EndElement)
        {
            lvl -= 1;
        }

    }

    // sloupec s typeToken je skryty, elementy poznam podle tucneho pisma
    ui->tableWidget->setColumnHidden(1,true);

    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->setColumnWidth(0,20);

    if (xml.hasError())
    {
        QMessageBox::critical(this, "Read XML: ERROR", xml.errorString(), QMessageBox::Ok);
    }
    else
    {

    }

}

void xsd::on_action_Export_XSD_schema_triggered()
{
    QString stylesheet = "";

    // definice hlavicky XSLT + 1. sablona, ktera je zatim pro vsechny pripady stejna
    stylesheet = "<?xml version='1.0' encoding='UTF-8'?>";
    stylesheet += "<xsl:stylesheet xmlns:xsl='http://www.w3.org/1999/XSL/Transform'";
    stylesheet += " xmlns:xs='http://www.w3.org/2001/XMLSchema'";
    stylesheet += " exclude-result-prefixes='xs'";
    stylesheet += " version='2.0'>";
    stylesheet += "<xsl:output omit-xml-declaration='yes' indent='yes'/>";
    stylesheet += "<xsl:strip-space elements='*'/> <!-- odstrani prazdne radky -->";
    stylesheet += "<xsl:template match='node()|@*'> <!-- nejdrive cele schema zkopirujeme -->";
    stylesheet += "<xsl:copy><xsl:apply-templates select='node()|@*'/></xsl:copy>";
    stylesheet += "</xsl:template>";

    int row = ui->tableWidget->rowCount();
    int col = ui->tableWidget->columnCount();

    // zde se budou nachazet jednotlive funkce pro tvorbu xslt sablon
    // zatim jen pro odstraneni odskrtnutych elementu
    XSLT_element_remove(stylesheet, row, col);

    // pote spoustim xslt proces
    XSLT_run(stylesheet);

}

void xsd::XSLT_run(QString stylesheet){

    // pokud spoustim xslt, musim mit ukonceny stylesheet
    stylesheet += "</xsl:stylesheet>";

    QFile newXSD(":/files/examples/output.xsd");

    newXSD.open(QIODevice::WriteOnly);
    qDebug() << newXSD.open(QIODevice::WriteOnly); // !!! neumim zapisovat do Qt Resources

    QXmlQuery query(QXmlQuery::XSLT20);

    // zdrojovy xsd - nutno propojit s radkem 39 (aby byly stejny)
    query.setFocus(QUrl("qrc:/files/examples/my1example.xsd"));
    //query.setFocus(QUrl("qrc:/files/examples/my2example.xsd"));

    // vygenerovany xslt stylesheet, podle ktereho transformuji
    query.setQuery(stylesheet);

    // nova verze XSD schematu po transformaci
    query.evaluateTo(&newXSD);

    qDebug() << "XSLT_run -> ";
    qDebug() << stylesheet;

}

void xsd::XSLT_element_remove(QString &stylesheet, int row, int col){

    QStringList element;  // cesta ke konkretnimu elementu
    QStringList elements; // vsechny cesty k elementum, ktere chci smazat
    QString tokenType = "";

    int ind = 0;
    row -= 1; // nepracuji s poslednim radkem (je tam navic, nevim proc !!!)

    for(int i=0; i<row; i++){

        QCheckBox *checkbox = qobject_cast<QCheckBox*>(ui->tableWidget->cellWidget(i,0));
        QTableWidgetItem *itm = ui->tableWidget->item(i,1);
        if(itm) tokenType = itm->text();

        if(tokenType == "element"){
            for(int j=2; j<col; j++){
                QTableWidgetItem *itm = ui->tableWidget->item(i,j);
                if(itm){
                    if(itm->column()<ind){
                        // pokud potrebuji, vratim se o uroven vyse
                        element.erase(element.end()-(ind-itm->column())-1,element.end());
                    }
                    element << itm->text();
                    ind = itm->column();
                    break;
                }
            }

            if(!checkbox->isChecked()){
                elements << element.join("/");
            }
        }
    }

    if(!elements.isEmpty()){
        stylesheet += "<xsl:template match='" + elements.join("|") + "' />";
        qDebug() << "XSLT_element_remove -> ";
        qDebug() << "<xsl:template match='" + elements.join("|") + "' />";
    }
}

/* NASTREL ZAPISU DO XML - QXmlStreamWriter (nebudu nakonec potrebovat)
QFile file("/home/bezdeka/Documents/cpp/XSDschema/output.xml");
file.open(QIODevice::WriteOnly);

QXmlStreamWriter xml(&file);

xml.setAutoFormatting(true);
xml.writeStartDocument();
...
*/
