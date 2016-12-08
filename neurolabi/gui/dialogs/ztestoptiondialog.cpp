#include "ztestoptiondialog.h"
#include "ui_ztestoptiondialog.h"

ZTestOptionDialog::ZTestOptionDialog(QWidget *parent) :
  QDialog(parent),
  ui(new Ui::ZTestOptionDialog)
{
  ui->setupUi(this);
}

ZTestOptionDialog::~ZTestOptionDialog()
{
  delete ui;
}

ZTestOptionDialog::EOption ZTestOptionDialog::getOption() const
{
  if (ui->normalTestRadioButton->isChecked()) {
    return OPTION_NORMAL;
  }

  if (ui->stressTestRadioButton->isChecked()) {
    return OPTION_STRESS;
  }

  return OPTION_NORMAL;
}