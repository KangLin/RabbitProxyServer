#include "Parameter.h"
#include "ui_Parameter.h"

CParameter::CParameter(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CParameter)
{
    ui->setupUi(this);
}

CParameter::~CParameter()
{
    delete ui;
}
