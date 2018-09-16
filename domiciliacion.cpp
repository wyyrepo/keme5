/* ----------------------------------------------------------------------------------
    KEME-Contabilidad; aplicación para llevar contabilidades

    Copyright (C)

    Este programa es software libre: usted puede redistribuirlo y/o modificarlo 
    bajo los términos de la Licencia Pública General GNU publicada 
    por la Fundación para el Software Libre, ya sea la versión 3 
    de la Licencia, o (a su elección) cualquier versión posterior.

    Este programa se distribuye con la esperanza de que sea útil, pero 
    SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita 
    MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
    Consulte los detalles de la Licencia Pública General GNU para obtener 
    una información más detallada. 

    Debería haber recibido una copia de la Licencia Pública General GNU 
    junto a este programa. 
    En caso contrario, consulte <http://www.gnu.org/licenses/>.
  ----------------------------------------------------------------------------------*/

#include "domiciliacion.h"
#include "buscasubcuenta.h"
#include "basedatos.h"
#include "filtrar_venci.h"
#include "directorio.h"
#include "pidefecha.h"
#include "tabla_asientos.h"
#include <QMessageBox>
#include <QFileDialog>

CustomSqlModel_dom::CustomSqlModel_dom(QObject *parent)
        : QSqlTableModel(parent)
    {
     comadecimal=haycomadecimal(); decimales=haydecimales();
    }

QString CustomSqlModel_dom::nombre_columna(int columna)
{
    return QSqlQueryModel::headerData(columna, Qt::Horizontal, Qt::DisplayRole).toString();
}

QVariant CustomSqlModel_dom::datagen(const QModelIndex &index, int role) const
    {
        QVariant value = QSqlQueryModel::data(index, role);
        return value;
    }

QVariant CustomSqlModel_dom::data(const QModelIndex &index, int role) const
    {
        QString vacio;
        QVariant value = QSqlQueryModel::data(index, role);
        if (value.isValid() && role == Qt::DisplayRole) {
            if (index.column() == 2 || index.column() == 5)
               {
                return value.toDate().toString(Qt::SystemLocaleDate);
               }
            if (index.column() == 3)
               {
                if (value.toDouble()<0.0001 && value.toDouble()>-0.0001)
                   return value.fromValue(vacio);
                else
                      {
                        return value.fromValue(formateanumerosep(value.toDouble(),comadecimal, decimales));
                      }
               }
            if (index.column() == 6)
               {
                if (value.toBool()) return value.fromValue(tr("DERECHO"));
                   else return value.fromValue(tr("OBLIGACION"));
               }
            if (index.column() == 7 || index.column() ==8 || index.column() ==14)
               {
                if (value.toBool()) return value.fromValue(tr("SI"));
                   else return value.fromValue(tr("NO"));
               }
        }
        if (role == Qt::TextAlignmentRole &&
             (index.column() == 0 || index.column()==3 ))
            return qVariantFromValue(int(Qt::AlignVCenter | Qt::AlignRight));
        if (role == Qt::TextAlignmentRole &&
             (index.column() != 0 && index.column()!=3 ))
            return qVariantFromValue(int(Qt::AlignVCenter | Qt::AlignCenter));
        return value;
    }




domiciliacion::domiciliacion(QString qusuario) : QDialog() {
    ui.setupUi(this);


CustomSqlModel_dom *elmodelo = new CustomSqlModel_dom;
model = elmodelo;
usuario=qusuario;

ui.emisiondateEdit->setDate(QDate::currentDate());
ui.cargodateEdit->setDate(QDate::currentDate());

 model->setTable("vencimientos");
 model->setHeaderData(0, Qt::Horizontal, tr("Núm."));
 model->setHeaderData(1, Qt::Horizontal, tr("Cuenta"));
 model->setHeaderData(2, Qt::Horizontal, tr("Fecha"));
 model->setHeaderData(3, Qt::Horizontal, tr("Importe"));
 model->setHeaderData(4, Qt::Horizontal, tr("Tesorería"));
 model->setHeaderData(5, Qt::Horizontal, tr("Vencimiento"));
 model->setHeaderData(6, Qt::Horizontal, tr("Tipo"));
 ui.tableView->setEditTriggers (QAbstractItemView::NoEditTriggers);
 // ui.tableView->setEditable(FALSE);
 ui.tableView->setSortingEnabled(true);

 // ui.tableView->setModel(model);
 ui.tableView->setAlternatingRowColors ( true);


 ui.tableView->setColumnHidden(6,true);
 // ui.tableView->setColumnHidden(7,true);
 ui.tableView->setColumnHidden(8,true);
 ui.tableView->setColumnHidden(9,true);
 ui.tableView->setColumnHidden(10,true);
 // ui.tableView->setColumnHidden(11,true);
 ui.tableView->setColumnHidden(12,true);
 ui.tableView->setColumnHidden(13,true);
 //ui.tableView->setColumnHidden(14,true);
 ui.tableView->setColumnHidden(15,true);

 comadecimal=haycomadecimal(); decimales=haydecimales();
 // model->select();

 // refrescar();

 ui.tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
 ui.tableView->setSelectionMode(QAbstractItemView::ExtendedSelection);

 ui.tableView->sortByColumn(0,Qt::AscendingOrder);

 ui.impagadopushButton->setEnabled(false);
 ui.cobropushButton->setEnabled(false);

// --------------------------------------------------------------
connect(ui.codigopushButton,SIGNAL(clicked()),this,SLOT(botoncodigo_pulsado()));
connect(ui.aux_bancolineEdit,SIGNAL(textChanged(QString)),this,SLOT(datos_banco()));
connect(ui.cargarpushButton,SIGNAL(clicked()),this,SLOT(cargar()));
connect(ui.tableView,SIGNAL(clicked(QModelIndex)),this,SLOT(infocuentalin(QModelIndex)));
connect(ui.borrapushButton,SIGNAL(clicked()),this,SLOT(borrar_seleccion()));
connect(ui.salirpushButton,SIGNAL(clicked()),this,SLOT(salir()));
connect(ui.aux_bancolineEdit,SIGNAL(editingFinished()),this,SLOT(auxbanco_finedic()));
connect(ui.c19pushButton,SIGNAL(clicked()),this,SLOT(c19()));
connect(ui.c58pushButton,SIGNAL(clicked()),this,SLOT(c58()));
connect(ui.sepa1914pushButton,SIGNAL(clicked()),this,SLOT(sepa19_14()));
connect(ui.c19checkBox,SIGNAL(stateChanged(int)),this,SLOT(check19_cambiado()));
connect(ui.c58checkBox,SIGNAL(stateChanged(int)),this,SLOT(check58_cambiado()));
connect(ui.sepa1914checkBox,SIGNAL(stateChanged(int)),this,SLOT(checksepa1914cambiado()));
connect(ui.cobropushButton,SIGNAL(clicked()),this,SLOT(registrar_cobro()));
connect(ui.procesadacheckBox,SIGNAL(stateChanged(int)),SLOT(checkprocesado()));
connect(ui.impagadopushButton, SIGNAL(clicked()),this,SLOT(impagados()));
connect(ui.informepushButton, SIGNAL(clicked()),this,SLOT(imprime()));
guardado=false;
}


void domiciliacion::botoncodigo_pulsado()
{
    QString cadena2;
    buscasubcuenta *labusqueda=new buscasubcuenta(ui.aux_bancolineEdit->text());
    labusqueda->exec();
    cadena2=labusqueda->seleccioncuenta();
    if (cadena2.length()>0) ui.aux_bancolineEdit->setText(cadena2);
       else ui.aux_bancolineEdit->clear();
    delete(labusqueda);

}


void domiciliacion::datos_banco()
{
   if (!esauxiliar(ui.aux_bancolineEdit->text())) return;
   QString ccc, iban, cedente1, cedente2, nriesgo, sufijo;
   ui.descrip_bancolineEdit->setText(descripcioncuenta(ui.aux_bancolineEdit->text()));
   basedatos::instancia()->datos_cuenta_banco(ui.aux_bancolineEdit->text(), &ccc, &iban,
                                   &cedente1, &cedente2, &nriesgo, &sufijo);
   ui.ccclineEdit->setText(ccc);
   ui.ibanlineEdit->setText(iban);
   ui.cedente1lineEdit->setText(cedente1);
   ui.cedente2lineEdit->setText(cedente2);
   ui.nriesgolineEdit->setText(nriesgo);
   ui.sufijolineEdit->setText(sufijo);

}


void domiciliacion::cargar()
{
  filtrar_venci *f = new filtrar_venci(ui.aux_bancolineEdit->text());
  f->exec();
  if (ui.id_domiciliacionlineEdit->text().isEmpty())
      asigna_id_remesa();
  f->asigna_id_domiciliacion(ui.id_domiciliacionlineEdit->text());
  delete f;
  refrescar();
}

void domiciliacion::asigna_id_remesa()
{
 basedatos::instancia()->transac_bloquea_config();
 qlonglong vnumdoc=basedatos::instancia()->select_prox_domiciliacion();
 QString cadnumdoc;
 cadnumdoc.setNum(vnumdoc);
 ui.id_domiciliacionlineEdit->setText(cadnumdoc);
 vnumdoc++;
 cadnumdoc.setNum(vnumdoc);
 basedatos::instancia()->updateConfiguracionprox_domiciliacion(cadnumdoc);
 basedatos::instancia()->desbloquea_y_commit();
 guarda_cabecera_remesa();

}


void domiciliacion::guarda_cabecera_remesa()
{
  basedatos::instancia()->guardadomiciliacion(ui.id_domiciliacionlineEdit->text(),
                                              ui.aux_bancolineEdit->text(),
                                              ui.descriplineEdit->text(),
                                              ui.ccclineEdit->text(),
                                              ui.ibanlineEdit->text(),
                                              ui.sufijolineEdit->text(),
                                              ui.cedente1lineEdit->text(),
                                              ui.cedente2lineEdit->text(),
                                              ui.nriesgolineEdit->text(),
                                              ui.emisiondateEdit->date(),
                                              ui.cargodateEdit->date(),                                              
                                              ui.c19checkBox->isChecked(),
                                              ui.c58checkBox->isChecked(),
                                              ui.sepa1914checkBox->isChecked(),
                                              ui.procesadacheckBox->isChecked());

}


domiciliacion::~domiciliacion()
{
    if (!guardado) salir();
}

void domiciliacion::salir()
{
    if (ui.id_domiciliacionlineEdit->text().isEmpty())
        asigna_id_remesa();
    else
      guarda_cabecera_remesa();
    guardado=true;
    accept();
}

void domiciliacion::refrescar()
{
 // QSqlQuery p=model->query();
 // p.exec();
 // model->clear();
 // model->setTable("vencimientos");
 asignafiltro();
 // model->clear();
 // model->select();
 calculasuma();
 ui.tableView->setCurrentIndex(model->index(0,0));
 infocuentalin(ui.tableView->currentIndex());
}

void domiciliacion::asignafiltro()
{
  QString cadfiltro="domiciliacion=";
  cadfiltro+=ui.id_domiciliacionlineEdit->text();
  model->setFilter (cadfiltro);
  model->select();
  ui.tableView->setModel(model);
  ui.tableView->setColumnHidden(6,true);
  // ui.tableView->setColumnHidden(7,true);
  ui.tableView->setColumnHidden(8,true);
  ui.tableView->setColumnHidden(9,true);
  ui.tableView->setColumnHidden(10,true);
  // ui.tableView->setColumnHidden(11,true);
  ui.tableView->setColumnHidden(12,true);
  ui.tableView->setColumnHidden(13,true);
  //ui.tableView->setColumnHidden(14,true);
  ui.tableView->setColumnHidden(15,true);

  ui.tableView->setColumnWidth(0,60);
  ui.tableView->setColumnWidth(7,70);
  ui.tableView->setColumnWidth(11,170);
  ui.tableView->setColumnWidth(14,70);
  ui.tableView->show();
  // QMessageBox::information( this, tr("Vencimientos"),cadfiltro);

}

void domiciliacion::calculasuma()
{
    double total = basedatos::instancia()->selectSumimportevendimientosfiltroderecho( model->filter() , true);

    total -= basedatos::instancia()->selectSumimportevendimientosfiltroderecho( model->filter() , false);

    double impagados = basedatos::instancia()->selectSumimpagados (model->filter(), true);

    ui.sumalineEdit->setText(formateanumero(total,comadecimal,decimales));
    ui.impagadoslineEdit->setText(formateanumero(impagados,comadecimal,decimales));
    ui.diferencialineEdit->setText(formateanumero(total-impagados,comadecimal,decimales));
}


void domiciliacion::infocuentalin(QModelIndex primaryKeyIndex)
{
   if (!primaryKeyIndex.isValid())
      {
       ui.codigolabel->setText("");
       ui.descriplabel->setText("");
      }
   int fila=primaryKeyIndex.row();
   QString cad = model->data(model->index(fila,1),Qt::DisplayRole).toString();
   ui.codigolabel->setText(cad);
   ui.descriplabel->setText(descripcioncuenta(cad));
}

void domiciliacion::borrar_seleccion()
{
    QItemSelectionModel *seleccion;
    seleccion=ui.tableView->selectionModel();
    if (seleccion->hasSelection())
       {
         QList<QModelIndex> listanum;
         listanum= seleccion->selectedRows ( 0 );
         for (int i = 0; i < listanum.size(); ++i)
              {
                QString cadena=model->datagen(listanum.at(i),Qt::DisplayRole).toString();
                basedatos::instancia()->pasa_domiciliacion_vto(cadena, "0");
              }
          refrescar();
          return;
       }
}


void domiciliacion::carga_domiciliacion(QString id)
{
    ui.procesadacheckBox->disconnect(SIGNAL(stateChanged(int)));
    ui.id_domiciliacionlineEdit->setText(id);
    QSqlQuery q=basedatos::instancia()->carga_domiciliacion(id);
    if (q.isActive())
        if (q.next())
           {
            // select aux_banco, descrip, ccc, iban, "
            //"sufijo, cedente1, cedente2, riesgo, fecha_conf, fecha_cargo, "
            //"emitido19, emitido58, procesada from domiciliaciones where id_remesa="
            ui.aux_bancolineEdit->setText(q.value(0).toString());
            ui.descriplineEdit->setText(q.value(1).toString());
            ui.ccclineEdit->setText(q.value(2).toString());
            ui.ibanlineEdit->setText(q.value(3).toString().trimmed());
            ui.sufijolineEdit->setText(q.value(4).toString().trimmed());
            ui.cedente1lineEdit->setText(q.value(5).toString().trimmed());
            ui.cedente2lineEdit->setText(q.value(6).toString().trimmed());
            ui.nriesgolineEdit->setText(q.value(7).toString().trimmed());
            ui.emisiondateEdit->setDate(q.value(8).toDate());
            ui.cargodateEdit->setDate(q.value(9).toDate());
            ui.c19checkBox->setChecked(q.value(10).toBool());
            ui.c58checkBox->setChecked(q.value(11).toBool());
            ui.sepa1914checkBox->setChecked(q.value(12).toBool());
            ui.procesadacheckBox->setChecked(q.value(13).toBool());
            refrescar();
           }
    check19_cambiado();
    check58_cambiado();
    checkprocesado();
    connect(ui.procesadacheckBox,SIGNAL(stateChanged(int)),SLOT(checkprocesado()));
    if (!ui.c19checkBox->isChecked() && !ui.c58checkBox->isChecked() && !ui.sepa1914checkBox->isChecked())
        ui.cobropushButton->setEnabled(false);

}


void domiciliacion::auxbanco_finedic()
{
  ui.aux_bancolineEdit->setText(expandepunto(ui.aux_bancolineEdit->text(),anchocuentas()));
  // if (!existesubcuenta(ui.aux_bancolineEdit->text())) ui.aux_bancolineEdit->clear();;
}


void domiciliacion::c19()
{
    if (ui.aux_bancolineEdit->text().isEmpty() || !existesubcuenta(ui.aux_bancolineEdit->text()))
       {
        QMessageBox::warning( this, tr("Domiciliaciones"),
                              tr("ERROR: no existe la cuenta auxiliar del banco"));
        return;
       }


    QString nombre;

  #ifdef NOMACHINE
    directorio *dir = new directorio();
    dir->pasa_directorio(dirtrabajobd());
    dir->filtrar("*.sop");
    dir->activa_pide_archivo("");
    if (dir->exec() == QDialog::Accepted)
      {
        nombre=dir->nuevo_conruta();
      }
     delete(dir);

  #else

    QFileDialog dialogofich(this);
    dialogofich.setFileMode(QFileDialog::AnyFile);
    dialogofich.setConfirmOverwrite ( false );
    dialogofich.setAcceptMode (QFileDialog::AcceptSave );
    dialogofich.setLabelText ( QFileDialog::LookIn, tr("Directorio:") );
    dialogofich.setLabelText ( QFileDialog::FileName, tr("Archivo:") );
    dialogofich.setLabelText ( QFileDialog::FileType, tr("Tipo de archivo:") );
    dialogofich.setLabelText ( QFileDialog::Accept, tr("Aceptar") );
    dialogofich.setLabelText ( QFileDialog::Reject, tr("Cancelar") );

    QStringList filtros;
    filtros << tr("Archivos con domiciliaciones bancarias (*.sop)");
    dialogofich.setNameFilters(filtros);
    dialogofich.setDirectory(adapta(dirtrabajobd()));
    dialogofich.setWindowTitle(tr("ARCHIVO DOMICILIACIÓN BANCARIA"));
    QStringList fileNames;
    if (dialogofich.exec())
       {
        fileNames = dialogofich.selectedFiles();
        if (fileNames.at(0).length()>0)
            {
             // QString nombre=nombre.fromLocal8bit(fileNames.at(0));
             nombre=fileNames.at(0);
            }
       }

  #endif

    if (nombre.isEmpty()) return;

    if (nombre.right(4)!=".sop") nombre=nombre+".sop";
     QFile estado( adapta(nombre)  );
     if (estado.exists() && QMessageBox::question(this,
         tr("¿ Sobreescribir ? Domiciliaciones"),
         tr("'%1' ya existe."
            "¿ Desea sobreescribirlo ?")
           .arg( nombre ),
         tr("&Sí"), tr("&No"),
         QString::null, 0, 1 ) )
       return ;


     QFile fichero( adapta(nombre)  );

     if ( !fichero.open( QIODevice::WriteOnly ) ) return;

     QTextStream stream( &fichero );
     // stream.setCodec("UTF-8");
     stream.setCodec("ISO 8859-1");

     // CABECERA DE PRESENTADOR
     stream << "5180";
     QString nif=completacadnum(basedatos::instancia()->cif(),9);
     QString sufijo=completacadnum(ui.sufijolineEdit->text().left(3),3);
     QString cadfecha_actual=ui.emisiondateEdit->date().toString("ddMMyy");
     stream << nif << sufijo << cadfecha_actual;
     QString str; str.fill(' ', 6);
     stream << str;

     // nombre del presentador
     QString nombred=completacadcad(adaptacad_aeat(nombreempresa()),40);
     stream << nombred;
     str.clear(); str.fill(' ',20);
     stream << str;

     // entidad receptora y oficina
     QString entidad=ui.ccclineEdit->text().left(4);
     QString oficina=ui.ccclineEdit->text().mid(5,4);
     stream << entidad << oficina;

     str.clear(); str.fill(' ',12);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     str.clear(); str.fill(' ',14);
     stream << str;

     stream << "\r\n";

     // CABECERA DE ORDENANTE
     stream << "5380";
     stream << nif << sufijo << cadfecha_actual;
     QString cadfechacargo=ui.cargodateEdit->date().toString("ddMMyy");
     stream << cadfechacargo;
     stream << nombred;
     // ccc de abono
     // 01234567890123456789012
     // 1234-1234-12-1234567890
     stream << ui.ccclineEdit->text().left(4); // entidad
     stream << ui.ccclineEdit->text().mid(5,4); // oficina
     stream << ui.ccclineEdit->text().mid(10,2); // DC
     stream << ui.ccclineEdit->text().right(10); // cuenta

     str.clear(); str.fill(' ',8);
     stream << str;

     stream << "01";

     str.clear(); str.fill(' ',10);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     str.clear(); str.fill(' ',14);
     stream << str;

     stream << "\r\n";

     // REGISTRO INDIVIDUAL OBLIGATORIO
     // Tenemos que hacer un select de vencimientos por domiciliación
     // con los campos necesarios
     QSqlQuery q = basedatos::instancia()->datos_domiciliacion(ui.id_domiciliacionlineEdit->text());

     bool activa_warning_noccc=false;
     double total_importe=0;
     int registros=0;
     if (q.isActive())
        {
         while (q.next())
           {
             // "select v.num, v.cta_ordenante, v.importe, v.fecha_operacion,"
             // "v.fecha_vencimiento, d.documento, datos.razon, datos.cif,"
             // "datos.ccc, datos.IBAN "
             stream << "5680";
             stream << nif << sufijo;
             QString referencia=completacadcad(q.value(1).toString(),12); // cuenta
             stream << referencia;
             QString cadtitular=completacadcad(adaptacad_aeat(q.value(6).toString()),40);
             stream << cadtitular;
             // ccc de adeudo
             // 01234567890123456789012
             // 1234-1234-12-1234567890
             QString ccc=q.value(8).toString();
             if (ccc.startsWith("-") || ccc.startsWith(" ")) activa_warning_noccc=true;
             stream << ccc.left(4); // entidad
             stream << ccc.mid(5,4); // oficina
             stream << ccc.mid(10,2); // DC
             stream << ccc.right(10); // cuenta

             QString cadimporte=formateanumero(q.value(2).toDouble(),comadecimal,decimales);
             total_importe+=q.value(2).toDouble();
             cadimporte.remove(",");
             cadimporte.remove(".");
             cadimporte=completacadnum(cadimporte,10);
             stream << cadimporte;

             str.clear(); str.fill(' ',16);
             stream << str;

             QString concepto;
             if (q.value(12).toString().isEmpty())
                {
                 concepto="Doc. ";
                 concepto+=q.value(5).toString();
                 concepto+=" - op. ";
                 concepto+=q.value(3).toDate().toString("dd-MM-yyyy");
                }
                else concepto=q.value(12).toString();

             concepto=completacadcad(adaptacad_aeat(concepto),40);

             stream << concepto;

             str.clear(); str.fill(' ',8);
             stream << str;

             stream << "\r\n";

             registros++;
           }
        }

     if (activa_warning_noccc)
        {
         QMessageBox::warning( this, tr("Cuaderno 19"),
                               tr("Advertencia: hay códigos de cuentas bancarias vacíos"));

        }

     // registro TOTAL ORDENATE
     stream << "5880";
     stream << nif << sufijo;

     str.clear(); str.fill(' ',12);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     str.clear(); str.fill(' ',20);
     stream << str;

     QString cadtotal; cadtotal.setNum(total_importe,'f',2);
     cadtotal.remove(".");
     cadtotal=completacadnum(cadtotal,10);
     stream << cadtotal;

     str.clear(); str.fill(' ',6);
     stream << str;

     // número de domiciliaciones
     QString cadnumdom; cadnumdom.setNum(registros); cadnumdom.remove(' ');
     cadnumdom=completacadnum(cadnumdom,10);
     stream << cadnumdom;

     // número de registros ordenante
     QString cadnumreg_ord; cadnumreg_ord.setNum(registros+2); cadnumreg_ord.remove(' ');
     cadnumreg_ord=completacadnum(cadnumreg_ord,10);
     stream << cadnumreg_ord;

     str.clear(); str.fill(' ',20);
     stream << str;

     str.clear(); str.fill(' ',18);
     stream << str;

     stream << "\r\n";

     // Registro TOTAL GENERAL
     stream << "5980";
     stream << nif << sufijo;

     str.clear(); str.fill(' ',12);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     // Número de ordenantes
     QString numordenantes=completacadnum("1",4);
     stream << numordenantes;

     str.clear(); str.fill(' ',16);
     stream << str;

     stream << cadtotal;

     str.clear(); str.fill(' ',6);
     stream << str;

     stream << cadnumdom;

     // número total de registros
     QString cadnumreg_tot; cadnumreg_tot.setNum(registros+4); cadnumreg_tot.remove(' ');
     cadnumreg_tot=completacadnum(cadnumreg_tot,10);
     stream << cadnumreg_tot;


     str.clear(); str.fill(' ',20);
     stream << str;

     str.clear(); str.fill(' ',18);
     stream << str;

     stream << "\r\n";

     fichero.close();

     QMessageBox::information( this, tr("Cuaderno 19"),
                               tr("Se ha terminado el proceso de generar el archivo"));
     ui.c19checkBox->setChecked(true);
     check19_cambiado();
}




void domiciliacion::c58()
{
    if (ui.aux_bancolineEdit->text().isEmpty() || !existesubcuenta(ui.aux_bancolineEdit->text()))
       {
        QMessageBox::warning( this, tr("Domiciliaciones"),
                              tr("ERROR: no existe la cuenta auxiliar del banco"));
        return;
       }

    QString nombre;

  #ifdef NOMACHINE
    directorio *dir = new directorio();
    dir->pasa_directorio(dirtrabajobd());
    dir->filtrar("*.sop");
    dir->activa_pide_archivo("");
    if (dir->exec() == QDialog::Accepted)
      {
        nombre=dir->nuevo_conruta();
      }
     delete(dir);

  #else

    QFileDialog dialogofich(this);
    dialogofich.setFileMode(QFileDialog::AnyFile);
    dialogofich.setConfirmOverwrite ( false );
    dialogofich.setAcceptMode (QFileDialog::AcceptSave );
    dialogofich.setLabelText ( QFileDialog::LookIn, tr("Directorio:") );
    dialogofich.setLabelText ( QFileDialog::FileName, tr("Archivo:") );
    dialogofich.setLabelText ( QFileDialog::FileType, tr("Tipo de archivo:") );
    dialogofich.setLabelText ( QFileDialog::Accept, tr("Aceptar") );
    dialogofich.setLabelText ( QFileDialog::Reject, tr("Cancelar") );

    QStringList filtros;
    filtros << tr("Archivos con domiciliaciones bancarias (*.sop)");
    dialogofich.setNameFilters(filtros);
    dialogofich.setDirectory(adapta(dirtrabajobd()));
    dialogofich.setWindowTitle(tr("ARCHIVO DOMICILIACIÓN BANCARIA"));
    QStringList fileNames;
    if (dialogofich.exec())
       {
        fileNames = dialogofich.selectedFiles();
        if (fileNames.at(0).length()>0)
            {
             // QString nombre=nombre.fromLocal8bit(fileNames.at(0));
             nombre=fileNames.at(0);
            }
       }

  #endif

    if (nombre.isEmpty()) return;

    if (nombre.right(4)!=".sop") nombre=nombre+".sop";
     QFile estado( adapta(nombre)  );
     if (estado.exists() && QMessageBox::question(this,
         tr("¿ Sobreescribir ? Domiciliaciones"),
         tr("'%1' ya existe."
            "¿ Desea sobreescribirlo ?")
           .arg( nombre ),
         tr("&Sí"), tr("&No"),
         QString::null, 0, 1 ) )
       return ;


     QFile fichero( adapta(nombre)  );

     if ( !fichero.open( QIODevice::WriteOnly ) ) return;

     QTextStream stream( &fichero );
     // stream.setCodec("UTF-8");
     stream.setCodec("ISO 8859-1");

     // CABECERA DE PRESENTADOR
     stream << "5170";
     QString nif=completacadnum(basedatos::instancia()->cif(),9);
     QString sufijo=completacadnum(ui.sufijolineEdit->text().left(3),3);
     QString cadfecha_actual=ui.emisiondateEdit->date().toString("ddMMyy");
     stream << nif << sufijo << cadfecha_actual;
     QString str; str.fill(' ', 6);
     stream << str;

     // nombre del presentador
     QString nombred=completacadcad(adaptacad_aeat(nombreempresa()),40);
     stream << nombred;
     str.clear(); str.fill(' ',20);
     stream << str;

     // entidad receptora y oficina
     QString entidad=ui.ccclineEdit->text().left(4);
     QString oficina=ui.ccclineEdit->text().mid(5,4);
     stream << entidad << oficina;

     str.clear(); str.fill(' ',12);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     str.clear(); str.fill(' ',14);
     stream << str;

     stream << "\r\n";


     // CABECERA DE ORDENANTE
     stream << "5370";
     stream << nif << sufijo << cadfecha_actual;
     QString cadfechacargo=ui.cargodateEdit->date().toString("ddMMyy");
     stream << cadfechacargo;

     // str.clear(); str.fill(' ',6);
     // stream << str;

     stream << nombred;


     // ccc de abono
     // 01234567890123456789012
     // 1234-1234-12-1234567890
     stream << ui.ccclineEdit->text().left(4); // entidad
     stream << ui.ccclineEdit->text().mid(5,4); // oficina
     stream << ui.ccclineEdit->text().mid(10,2); // DC
     stream << ui.ccclineEdit->text().right(10); // cuenta

     str.clear(); str.fill(' ',8);
     stream << str;

     stream << "06";

     str.clear(); str.fill(' ',10);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     str.clear(); str.fill(' ',2);
     stream << str;

     // codigo INE plaza (NUMÉRICO)
     QString codine=completacadnum(basedatos::instancia()->config_codigo_ine_plaza(),9);
     stream << codine;

     str.clear(); str.fill(' ',3);
     stream << str;

     stream << "\r\n";


     // REGISTRO INDIVIDUAL OBLIGATORIO
     // Tenemos que hacer un select de vencimientos por domiciliación
     // con los campos necesarios
     QSqlQuery q = basedatos::instancia()->datos_domiciliacion(ui.id_domiciliacionlineEdit->text());

     bool activa_warning_noccc=false;
     double total_importe=0;
     int registros=0;
     if (q.isActive())
        {
         while (q.next())
           {
             // "select v.num, v.cta_ordenante, v.importe, v.fecha_operacion,"
             // "v.fecha_vencimiento, d.documento, datos.razon, datos.cif,"
             // "datos.ccc, datos.IBAN "
             stream << "5670";
             stream << nif << sufijo;
             QString referencia=completacadcad(q.value(1).toString(),12); // cuenta
             stream << referencia;
             QString cadtitular=completacadcad(adaptacad_aeat(q.value(6).toString()),40);
             stream << cadtitular;
             // ccc de adeudo
             // 01234567890123456789012
             // 1234-1234-12-1234567890
             QString ccc=q.value(8).toString();
             if (ccc.startsWith("-") || ccc.startsWith(" ")) activa_warning_noccc=true;
             stream << ccc.left(4); // entidad
             stream << ccc.mid(5,4); // oficina
             stream << ccc.mid(10,2); // DC
             stream << ccc.right(10); // cuenta

             QString cadimporte=formateanumero(q.value(2).toDouble(),comadecimal,decimales);
             total_importe+=q.value(2).toDouble();
             cadimporte.remove(",");
             cadimporte.remove(".");
             cadimporte=completacadnum(cadimporte,10);
             stream << cadimporte;

             // este sería el código para devoluciones (6) y código ref. interna (10)
             str.clear(); str.fill(' ',16);
             stream << str;

             QString concepto;
             if (q.value(12).toString().isEmpty())
                {
                 concepto="Doc. ";
                 concepto+=q.value(5).toString();
                 concepto+=" - op. ";
                 concepto+=q.value(3).toDate().toString("dd-MM-yyyy");
                }
                else concepto=q.value(12).toString();

             concepto=completacadcad(adaptacad_aeat(concepto),40);

             stream << concepto;

             // fecha de vencimiento
             QString cadfechavenci=q.value(4).toDate().toString("ddMMyy");
             stream << cadfechavenci;

             str.clear(); str.fill(' ',2);
             stream << str;

             stream << "\r\n";

             registros++;
           }
        }

     if (activa_warning_noccc)
        {
         QMessageBox::warning( this, tr("Cuaderno 19"),
                               tr("Advertencia: hay códigos de cuentas bancarias vacíos"));

        }


     // registro TOTAL ORDENATE
     stream << "5870";
     stream << nif << sufijo;

     str.clear(); str.fill(' ',12);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     str.clear(); str.fill(' ',20);
     stream << str;

     QString cadtotal; cadtotal.setNum(total_importe,'f',2);
     cadtotal.remove(".");
     cadtotal=completacadnum(cadtotal,10);
     stream << cadtotal;

     str.clear(); str.fill(' ',6);
     stream << str;

     // número de domiciliaciones
     QString cadnumdom; cadnumdom.setNum(registros); cadnumdom.remove(' ');
     cadnumdom=completacadnum(cadnumdom,10);
     stream << cadnumdom;

     // número de registros ordenante
     QString cadnumreg_ord; cadnumreg_ord.setNum(registros+2); cadnumreg_ord.remove(' ');
     cadnumreg_ord=completacadnum(cadnumreg_ord,10);
     stream << cadnumreg_ord;

     str.clear(); str.fill(' ',20);
     stream << str;

     str.clear(); str.fill(' ',18);
     stream << str;

     stream << "\r\n";

     // Registro TOTAL GENERAL
     stream << "5970";
     stream << nif << sufijo;

     str.clear(); str.fill(' ',12);
     stream << str;

     str.clear(); str.fill(' ',40);
     stream << str;

     // Número de ordenantes
     QString numordenantes=completacadnum("1",4);
     stream << numordenantes;

     str.clear(); str.fill(' ',16);
     stream << str;

     stream << cadtotal;

     str.clear(); str.fill(' ',6);
     stream << str;

     stream << cadnumdom;

     // número total de registros
     QString cadnumreg_tot; cadnumreg_tot.setNum(registros+4); cadnumreg_tot.remove(' ');
     cadnumreg_tot=completacadnum(cadnumreg_tot,10);
     stream << cadnumreg_tot;


     str.clear(); str.fill(' ',20);
     stream << str;

     str.clear(); str.fill(' ',18);
     stream << str;

     stream << "\r\n";

     fichero.close();

     QMessageBox::information( this, tr("Cuaderno 58"),
                               tr("Se ha terminado el proceso de generar el archivo"));
     ui.c58checkBox->setChecked(true);
     check58_cambiado();

 }


// -----------------------------------------------------------------------------------
void domiciliacion::sepa19_14()
{
    if (ui.aux_bancolineEdit->text().isEmpty() || !existesubcuenta(ui.aux_bancolineEdit->text()))
       {
        QMessageBox::warning( this, tr("Domiciliaciones"),
                              tr("ERROR: no existe la cuenta auxiliar del banco"));
        return;
       }


    QString nombre;

  #ifdef NOMACHINE
    directorio *dir = new directorio();
    dir->pasa_directorio(dirtrabajobd());
    dir->filtrar("*.sop");
    dir->activa_pide_archivo("");
    if (dir->exec() == QDialog::Accepted)
      {
        nombre=dir->nuevo_conruta();
      }
     delete(dir);

  #else

    QFileDialog dialogofich(this);
    dialogofich.setFileMode(QFileDialog::AnyFile);
    dialogofich.setConfirmOverwrite ( false );
    dialogofich.setAcceptMode (QFileDialog::AcceptSave );
    dialogofich.setLabelText ( QFileDialog::LookIn, tr("Directorio:") );
    dialogofich.setLabelText ( QFileDialog::FileName, tr("Archivo:") );
    dialogofich.setLabelText ( QFileDialog::FileType, tr("Tipo de archivo:") );
    dialogofich.setLabelText ( QFileDialog::Accept, tr("Aceptar") );
    dialogofich.setLabelText ( QFileDialog::Reject, tr("Cancelar") );

    QStringList filtros;
    filtros << tr("Archivos con domiciliaciones bancarias (*.sop)");
    dialogofich.setNameFilters(filtros);
    dialogofich.setDirectory(adapta(dirtrabajobd()));
    dialogofich.setWindowTitle(tr("ARCHIVO DOMICILIACIÓN BANCARIA"));
    QStringList fileNames;
    if (dialogofich.exec())
       {
        fileNames = dialogofich.selectedFiles();
        if (fileNames.at(0).length()>0)
            {
             // QString nombre=nombre.fromLocal8bit(fileNames.at(0));
             nombre=fileNames.at(0);
            }
       }

  #endif

    if (nombre.isEmpty()) return;

    if (nombre.right(4)!=".sop") nombre=nombre+".sop";
     QFile estado( adapta(nombre)  );
     if (estado.exists() && QMessageBox::question(this,
         tr("¿ Sobreescribir ? Domiciliaciones"),
         tr("'%1' ya existe."
            "¿ Desea sobreescribirlo ?")
           .arg( nombre ),
         tr("&Sí"), tr("&No"),
         QString::null, 0, 1 ) )
       return ;


     QFile fichero( adapta(nombre)  );

     if ( !fichero.open( QIODevice::WriteOnly ) ) return;

     QTextStream stream( &fichero );
     // stream.setCodec("UTF-8");
     stream.setCodec("ISO 8859-1");

     // CABECERA DE PRESENTADOR
     // ---------------------------------------------------------------------

     stream << "0119154001";

     QString sufijo=completacadnum(ui.sufijolineEdit->text().left(3),3);
     QString pais="ES";

     QString dig_control=digito_cotrol_IBAN(pais+"00"+basedatos::instancia()->cif());
     QString campo4=completacadcad(adaptacad_aeat(
             pais+dig_control+sufijo+basedatos::instancia()->cif()),35);
     stream << campo4;

     // nombre del presentador
     QString nombred=completacadcad(adaptacad_aeat(nombreempresa()),70);
     stream << nombred;

     // fecha de creación del fichero
     QString cadfecha_actual=ui.emisiondateEdit->date().toString("yyyyMMdd");
     stream << cadfecha_actual;

     // identificación del fichero
     QString campo7="PRE";
     QDate fecha_actual=QDate::currentDate();
     campo7+=fecha_actual.toString("yyyyMMdd");
     QString hora=QTime().currentTime().toString("hhmmss");
     campo7+=hora;
     campo7+="00000";
     campo7+=completacadcad("KEME2982",13);
     stream << campo7;

     // entidad receptora y oficina
     QString entidad=ui.ccclineEdit->text().left(4);
     QString oficina=ui.ccclineEdit->text().mid(5,4);
     stream << entidad << oficina;

     QString str; str.fill(' ', 434);
     stream << str;

     stream << "\r\n";

     // CABECERA DEL ACREEDOR
     // ---------------------------------------------------------------------
     stream << "0219154002";

     // identificador acreedor
     sufijo=completacadnum(ui.sufijolineEdit->text().left(3),3);
     pais="ES";

     dig_control=digito_cotrol_IBAN(pais+"00"+basedatos::instancia()->cif());
     campo4=completacadcad(adaptacad_aeat(
             pais+dig_control+sufijo+basedatos::instancia()->cif()),35);
     stream << campo4;

     // fecha de cobro
     QDate fecha_cobro=ui.cargodateEdit->date();
     // fecha_cobro=fecha_cobro.addDays(1);
     // QString cadfechacargo=ui.cargodateEdit->date().toString("yyyyMMdd");
     QString cadfechacargo=fecha_cobro.toString("yyyyMMdd");
     stream << cadfechacargo;

     // nombre del acreedor
     nombred=completacadcad(adaptacad_aeat(nombreempresa()),70);
     stream << nombred;

     QString domicilio=completacadcad(adaptacad_aeat(basedatos::instancia()->domicilio().left(50)),50);
     stream << domicilio;

     QString cp_localidad=basedatos::instancia()->cpostal()+" "+ basedatos::instancia()->ciudad();
     cp_localidad=completacadcad(adaptacad_aeat(cp_localidad.left(50)),50);
     stream << cp_localidad;

     QString provincia=completacadcad(adaptacad_aeat(basedatos::instancia()->provincia().left(40)),40);
     stream << provincia;

     // País
     stream << "ES";

     // IBAN acreedor
     QString iban=ui.ibanlineEdit->text().trimmed();
     stream << completacadcad(iban,34);

     str.clear(); str.fill(' ', 301);
     stream << str;

     stream << "\r\n";

     // REGISTROS INDIVIDUALES DE ADEUDOS


     // Tenemos que hacer un select de vencimientos por domiciliación
     // con los campos necesarios
     // int numregs= basedatos::instancia()->cuenta_regs_domiciliacion(ui.id_domiciliacionlineEdit->text());

     QSqlQuery q = basedatos::instancia()->datos_domiciliacion(ui.id_domiciliacionlineEdit->text());

     bool activa_warning_no_iban=false;
     bool activa_warning_no_bic=false;
     double total_importe=0;
     int registro=1;
     if (q.isActive())
        {
         while (q.next())
           {
             // "select v.num, v.cta_ordenante, v.importe, v.fecha_operacion,"
             // "v.fecha_vencimiento, d.documento, datos.razon, datos.cif,"
             // "datos.ccc, datos.IBAN, v.pendiente, v.impagado, v.concepto, datos.bic, "
             // "datos.domicilio, datos.codigopostal, datos.poblacion, datos.provincia, "
             // "datos.pais , datos.ref_mandato, datos.firma_mandato"
             stream << "0319154003";
             QString nif=q.value(7).toString().trimmed();
             QString at10=completacadcad(nif+sufijo,35);
             stream << at10;


             QString referencia;
             if (q.value(19).toString().isEmpty())
               referencia=completacadcad(q.value(1).toString(),35); // cuenta
              else
                 referencia=completacadcad(q.value(19).toString(),35);
             stream << referencia;

             QString at21;
             // if (registro==1 && numregs>1)
             //    at21="FRST";
             // if (registro==1 && numregs==1)
             // at21="OOFF";
             // if (registro>1 && numregs==registro)
             //     at21="FNAL";
             // if (registro>1 && numregs>registro)
             at21="RCUR";

             stream << at21;

             // categoría de propósito

             stream << "TRAD"; // Transacción relacionada con el pago de una transacción comercial

             // importe del adeudo
             QString cadimporte=formateanumero(q.value(2).toDouble(),comadecimal,decimales);
             total_importe+=q.value(2).toDouble();
             cadimporte.remove(",");
             cadimporte.remove(".");
             cadimporte=completacadnum(cadimporte,11);
             stream << cadimporte;

             // fecha de firma del mandato
             QDate fecha_firma=q.value(20).toDate();
             if (fecha_firma<QDate(2009,10,31)) fecha_firma=QDate(2009,10,31);
             stream << fecha_firma.toString("yyyyMMdd");

             // entidad del deudor (BIC)
             QString bic=q.value(13).toString().left(11);
             stream << completacadcad(bic,11);
             if (bic.isEmpty()) activa_warning_no_bic=true;

             // nombre del deudor
             QString cadtitular=completacadcad(adaptacad_aeat(q.value(6).toString()),70);
             stream << cadtitular;

             // direccion del deudor
             stream << completacadcad(adaptacad_aeat(q.value(14).toString().left(50)),50);

             // cp y localidad
             QString cp_localidad=adaptacad_aeat(q.value(15).toString().trimmed() + " " +q.value(16).toString());
             stream << completacadcad(cp_localidad,50);

             // provincia
             QString provincia=adaptacad_aeat(q.value(17).toString().trimmed());
             stream << completacadcad(provincia,40);

             // país
             stream << completacadcad(q.value(18).toString(),2);

             // campo 16: tipo de identificación 1-Organización  2-Persona
             QString qnif=q.value(7).toString();
             QString primera_letra_nif=qnif.left(1);
             QString pf="0123456789X";
             bool persona_fisica=pf.contains(primera_letra_nif);
             if (persona_fisica)
                stream << "2";
               else stream << "1";

             // campo 17
             if (persona_fisica)
               {
                 stream << "J";
                 stream << completacadcad(qnif,35);
               }
               else
                    {
                     stream << "I";
                     stream << completacadcad(qnif,35);
                    }

             // campo 18

             str.clear(); str.fill(' ',35);
             stream << str;

             // campo 19

             stream << "A"; // cuenta del deudor IBAN

             // campo 20  -- IBAN del deudor
             stream << completacadcad(q.value(9).toString(),34);
             if (q.value(9).toString().isEmpty())
                 activa_warning_no_iban=true;

             // campo 21 -- propósito del adeudo
             stream << "TRAD";

             // campo 22 -- concepto
             QString concepto;
             if (q.value(12).toString().isEmpty())
                {
                 concepto="Doc. ";
                 concepto+=q.value(5).toString();
                 concepto+=" - op. ";
                 concepto+=q.value(3).toDate().toString("dd-MM-yyyy");
                }
                else concepto=q.value(12).toString();

             concepto=completacadcad(adaptacad_aeat(concepto),140);

             stream << concepto;

             // campo 23 -- libre
             str.clear(); str.fill(' ',19);
             stream << str;


             stream << "\r\n";

             registro++;
           }
        }


     if (activa_warning_no_iban)
        {
         QMessageBox::warning( this, tr("Cuaderno 19"),
                               tr("Advertencia: hay códigos de cuentas bancarias IBAN vacíos"));

        }

     if (activa_warning_no_bic)
        {
         QMessageBox::warning( this, tr("Cuaderno 19"),
                               tr("Advertencia: hay códigos BIC vacíos"));

        }

     // -----------------------------------------------------------
     // registro de totales de acreedor por fecha de cobro
     // -----------------------------------------------------------

     stream << "04";

     // identificador acreedor
     sufijo=completacadnum(ui.sufijolineEdit->text().left(3),3);
     pais="ES";

     dig_control=digito_cotrol_IBAN(pais+"00"+basedatos::instancia()->cif());
     campo4=completacadcad(adaptacad_aeat(
             pais+dig_control+sufijo+basedatos::instancia()->cif()),35);
     stream << campo4;

     // fecha de cobro
     stream << cadfechacargo;

     // total de importes
     QString cadtotal=formateanumero(total_importe,comadecimal,decimales);
     cadtotal.remove(",");
     cadtotal.remove(".");
     stream << completacadnum(cadtotal,17);

     // número de registros individuales obligatorios (8 posiciones)
     QString cadnumregistros;
     cadnumregistros.setNum(registro-1);
     stream << completacadnum(cadnumregistros,8);

     // número total registros bloque acreedor
     cadnumregistros.setNum(registro+1);
     stream << completacadnum(cadnumregistros,10);

     // campo 7 -- libre
     str.clear(); str.fill(' ',520);
     stream << str;

     stream << "\r\n";

     // ----------------------------------------------
     // Registros de totales de acreedor
     // ----------------------------------------------
     stream << "05";

     // identificador acreedor
     sufijo=completacadnum(ui.sufijolineEdit->text().left(3),3);
     pais="ES";

     dig_control=digito_cotrol_IBAN(pais+"00"+basedatos::instancia()->cif());
     campo4=completacadcad(adaptacad_aeat(
             pais+dig_control+sufijo+basedatos::instancia()->cif()),35);
     stream << campo4;

     // total importes 17 posiciones
     stream << completacadnum(cadtotal,17);

     // número de adeudos
     cadnumregistros.setNum(registro-1);
     stream << completacadnum(cadnumregistros,8);

     // número total de registros
     cadnumregistros.setNum(registro+1+1);
     stream << completacadnum(cadnumregistros,10);

     // campo 6 -- libre
     str.clear(); str.fill(' ',528);
     stream << str;

     stream << "\r\n";

     // --------------------------------------------------
     // -- REGISTRO DE TOTALES GENERAL
     // --------------------------------------------------
     stream << "99";

     // total importes 17 posiciones
     stream << completacadnum(cadtotal,17);

     // número registros individuales
     cadnumregistros.setNum(registro-1);
     stream << completacadnum(cadnumregistros,8);

     // número total de registros
     cadnumregistros.setNum(registro+1+1+1+1); // se ha añadido uno por la cabecera
     stream << completacadnum(cadnumregistros,10);

     // campo 5 -- libre
     str.clear(); str.fill(' ',563);
     stream << str;

     stream << "\r\n";
     // AQUÍ NOS HEMOS QUEDADO


     fichero.close();

     QMessageBox::information( this, tr("Cuaderno 19"),
                               tr("Se ha terminado el proceso de generar el archivo"));
     ui.sepa1914checkBox->setChecked(true);
     ui.cobropushButton->setEnabled(true);
     ui.cargarpushButton->setEnabled(false);
     ui.borrapushButton->setEnabled(false);
     ui.c19pushButton->setEnabled(false);
     ui.c58pushButton->setEnabled(false);
     ui.sepa1914pushButton->setEnabled(false);
    //  ui.c19checkBox->setChecked(true);
    //  check19_cambiado();
}

// ---------------------------------------------------------------------------------

void domiciliacion::check19_cambiado()
{
 /*
  if (ui.c19checkBox->isChecked() && ui.c58checkBox->isChecked())
      ui.c58checkBox->setChecked(false);

  if (ui.c19checkBox->isChecked() && ui.sepa1914checkBox->isChecked())
      ui.sepa1914checkBox->setChecked(false);
*/

  if (ui.c19checkBox->isChecked() || ui.c58checkBox->isChecked()
          || ui.sepa1914checkBox->isChecked())
     {
      ui.cobropushButton->setEnabled(true);
      ui.cargarpushButton->setEnabled(false);
      ui.borrapushButton->setEnabled(false);
      ui.c19pushButton->setEnabled(false);
      ui.c58pushButton->setEnabled(false);
      ui.sepa1914pushButton->setEnabled(false);
     }

  if (!ui.c19checkBox->isChecked() && !ui.c58checkBox->isChecked()
      && !ui.sepa1914checkBox->isChecked())
    {
      ui.cobropushButton->setEnabled(false);
      ui.cargarpushButton->setEnabled(true);
      ui.borrapushButton->setEnabled(true);
      ui.c19pushButton->setEnabled(true);
      ui.c58pushButton->setEnabled(true);
      ui.sepa1914pushButton->setEnabled(true);
    }
}

void domiciliacion::checksepa1914cambiado()
{
    /*
    if (ui.sepa1914checkBox->isChecked())
       {
        ui.c58checkBox->setChecked(false);
        ui.c19checkBox->setChecked(false);
       }
   */
    if (ui.c19checkBox->isChecked() || ui.c58checkBox->isChecked()
            || ui.sepa1914checkBox->isChecked())
       {
        ui.cobropushButton->setEnabled(true);
        ui.cargarpushButton->setEnabled(false);
        ui.borrapushButton->setEnabled(false);
        ui.c19pushButton->setEnabled(false);
        ui.c58pushButton->setEnabled(false);
        ui.sepa1914pushButton->setEnabled(false);
       }

    if (!ui.c19checkBox->isChecked() && !ui.c58checkBox->isChecked()
        && !ui.sepa1914checkBox->isChecked())
      {
        ui.cobropushButton->setEnabled(false);
        ui.cargarpushButton->setEnabled(true);
        ui.borrapushButton->setEnabled(true);
        ui.c19pushButton->setEnabled(true);
        ui.c58pushButton->setEnabled(true);
        ui.sepa1914pushButton->setEnabled(true);
      }

}

void domiciliacion::check58_cambiado()
{
  /*
  if (ui.c19checkBox->isChecked() && ui.c58checkBox->isChecked())
      ui.c19checkBox->setChecked(false);

  if (ui.sepa1914checkBox->isChecked() && ui.c58checkBox->isChecked())
      ui.sepa1914checkBox->setChecked(false);
*/
  if (ui.c19checkBox->isChecked() || ui.c58checkBox->isChecked()
          || ui.sepa1914checkBox->isChecked())
     {
      ui.cobropushButton->setEnabled(true);
      ui.cargarpushButton->setEnabled(false);
      ui.borrapushButton->setEnabled(false);
      ui.c19pushButton->setEnabled(false);
      ui.c58pushButton->setEnabled(false);
      ui.sepa1914pushButton->setEnabled(false);
     }

  if (!ui.c19checkBox->isChecked() && !ui.c58checkBox->isChecked()
      && !ui.sepa1914checkBox->isChecked())
    {
      ui.cobropushButton->setEnabled(false);
      ui.cargarpushButton->setEnabled(true);
      ui.borrapushButton->setEnabled(true);
      ui.c19pushButton->setEnabled(true);
      ui.c58pushButton->setEnabled(true);
      ui.sepa1914pushButton->setEnabled(true);
    }
}


void domiciliacion::registrar_cobro()
{
    if (ui.aux_bancolineEdit->text().isEmpty() || !existesubcuenta(ui.aux_bancolineEdit->text()))
       {
        QMessageBox::warning( this, tr("Domiciliaciones"),
                              tr("ERROR: no existe la cuenta auxiliar del banco"));
        return;
       }

    pidefecha *p = new pidefecha();
    p->asignaetiqueta(tr("FECHA ABONO EN CUENTA"));
    p->pasafecha(ui.cargodateEdit->date());
    p->asigna_concepto(tr("Procesado de domiciliación ")+ui.descriplineEdit->text());
    int resultado=p->exec();
    QDate qvto;
    if (resultado == QDialog::Accepted) qvto=p->fecha();
       else return;
    QString ejercicio=ejerciciodelafecha(qvto);
    QString concepto_resumen=p->recupera_concepto();
    QString documento=p->recupera_documento();
    delete(p);
    if (!fechacorrespondeaejercicio(qvto) || fechadejerciciocerrado(qvto))
     {
        QMessageBox::warning( this, tr("Vencimientos"),tr("ERROR en unificar procesados: la fecha"
                                                        "suministrada para la selección no es correcta"));
        return;
     }

  if (basedatos::instancia()->fecha_periodo_bloqueado(qvto))
   {
    QMessageBox::warning( this, tr("Vencimientos"),tr("ERROR: La fecha del vencimiento corresponde "
                                                          "a un periodo bloqueado"));
    return;
   }

  if (basedatos::instancia()->existe_bloqueo_cta(ui.aux_bancolineEdit->text(),qvto))
  {
     QMessageBox::warning( this, tr("Vencimientos"),tr("ERROR: La cuenta %1 "
                                    "está bloqueada").arg(ui.aux_bancolineEdit->text()));
     return;
  }

  QSqlQuery q = basedatos::instancia()->datos_domiciliacion(ui.id_domiciliacionlineEdit->text());
  if (q.isActive())
      while (q.next())
        {
         if (basedatos::instancia()->existe_bloqueo_cta(
               q.value(1).toString(),qvto))
           {
            QMessageBox::warning( this, tr("Domiciliaciones"),tr("ERROR: La cuenta %1 "
                                "está bloqueada").arg(q.value(1).toString()));
            return;
           }

        }


  if (QMessageBox::question(
        this,
        tr("Vencimientos"),
        tr("Se va a procesar contablemente la domiciliación.\n¿Desea continuar?"),
        tr("&Sí"), tr("&No"),
        QString::null, 0, 1 ) )
        return ;


  QSqlDatabase::database().transaction();

  basedatos::instancia()->bloquea_para_listavenci();
  QString cadnumasiento;

  qlonglong vnum = basedatos::instancia()->proximoasiento(ejercicio);

  QString cadnum; cadnum.setNum(vnum+1);
  cadnumasiento.setNum(vnum);
  basedatos::instancia()->update_ejercicio_prox_asiento(ejercicio, cadnum);

  QString cadnumpase;
  qlonglong pase = basedatos::instancia()->selectProx_paseconfiguracion();
  // esto sería para cada pase del asiento; incrementar pase
  cadnumpase.setNum(pase);


  double saldo=0;

  q = basedatos::instancia()->datos_domiciliacion(ui.id_domiciliacionlineEdit->text());
  if (q.isActive())
   while (q.next())
     {
      // QMessageBox::warning( this, tr("Domiciliaciones"),tr("hola"));
      QString concepto=q.value(12).toString();
      QString debe, haber;
      QString cadimporte = q.value(2).toString();
          debe = "0";
          haber = cadimporte;
          saldo += cadimporte.toDouble();
      if (concepto.isEmpty())
          {
           concepto = tr("Domiciliación ");
           concepto += descripcioncuenta(q.value(1).toString());
           concepto += " de ";
           concepto += q.value(3).toDate().toString("dd-MM-yyyy");
          }
      QString pase_diario_vencimiento=cadnumpase;
      basedatos::instancia()->insertDiario10( cadnumasiento, cadnumpase, qvto.toString("yyyy-MM-dd"),
                                              q.value(1).toString() , debe, haber,
                                              concepto, documento, usuario, ejercicio);

     //-------------------------------------------------------------------------------------------------

          basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(q.value(1).toString(),
                                                                        cadimporte, false );
          // restar
     //-------------------------------------------------------------------------------------------------
     //-------------------------------------------------------------------------------------------------
     basedatos::instancia()->haz_venci_procesado(q.value(0).toString(),
                                                pase_diario_vencimiento);
     QString ccc, iban, cedente1, cedente2, nriesgo, sufijo;
     basedatos::instancia()->datos_cuenta_banco(ui.aux_bancolineEdit->text(),
                                                &ccc,
                                                &iban,
                                                &cedente1,
                                                &cedente2,
                                                &nriesgo,
                                                &sufijo);
    /* (QString cuenta_tesoreria, QDate fecha_realizacion,
      bool pendiente, QString pase_diario_vencimiento,
      QString medio_realizacion, QString cuenta_bancaria,
      QString num)*/
     basedatos::instancia()->updateVencimientos4num( ui.aux_bancolineEdit->text() ,
                                              qvto , false,cadnumpase,
                                              "",
                                              iban.isEmpty() ? ccc : iban,                                              
                                              q.value(0).toString() );

     //---------------------------------------------------------------------------------------------
     pase++;
     cadnumpase.setNum(pase);
    }

 // queda pasar la cuenta de tesorería
 QString debe, haber;
  if (saldo>0)
    {
        debe.setNum(saldo,'f',2);
        haber = "0";
    }
   else
    {
        debe = "0";
        haber.setNum(-saldo,'f',2);
    }

 basedatos::instancia()->insertDiario10( cadnumasiento, cadnumpase,
                                            qvto.toString("yyyy-MM-dd"),
                                            ui.aux_bancolineEdit->text(),
                                            debe, haber, concepto_resumen, documento,
                                            usuario,ejercicio );


//---------------------------------------------------------------------------------------------------------
// por aquí vamos
if (saldo>0) {
    QString cnum; cnum.setNum(saldo,'f',2);
    basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(ui.aux_bancolineEdit->text(),
                                                                  cnum, true);
    //sumar
}
else {
    if (saldo<0) saldo=-1*saldo;
    // restar
    QString cnum; cnum.setNum(saldo,'f',2);
    basedatos::instancia()->updateSaldossubcuentasaldocargocodigo(ui.aux_bancolineEdit->text(),
                                                                  cnum, false);
}

//--------------------------------------------------------------------------------------------------------
pase++;
cadnumpase.setNum(pase);
basedatos::instancia()->updateConfiguracionprox_pase(cadnumpase);

// fin de la transacción
basedatos::instancia()->desbloquea_y_commit();

// accept() ;
refrescar();


QMessageBox::information( this, tr("Domiciliación"),
                         tr("Se han procesado los registros en un asiento contable"));

ui.procesadacheckBox->setChecked(true);
ui.cuadernosgroupBox->setEnabled(false);
ui.cobropushButton->setEnabled(false);
ui.impagadopushButton->setEnabled(true);
}


void domiciliacion::checkprocesado()
{
  if (ui.procesadacheckBox->isChecked())
    {
      // activar --> solo posible si los vencimientos no están pendientes,
      // es decir, están todos procesados.
      if (model->rowCount()>0)
         {
          if (model->record(0).value("pendiente").toBool())
             {
              QMessageBox::warning( this, tr("Domiciliación"),
                                    tr("No es posible activar procesada. Los vencimientos no se han procesado"));
              ui.procesadacheckBox->disconnect(SIGNAL(stateChanged(int)));
              ui.procesadacheckBox->setChecked(false);
              connect(ui.procesadacheckBox,SIGNAL(stateChanged(int)),SLOT(checkprocesado()));
              return;
             }
         }
      ui.cuadernosgroupBox->setEnabled(false);
      ui.cobropushButton->setEnabled(false);
      ui.impagadopushButton->setEnabled(true);
    }
    else
        {
      // desactivar --> solo posible si los vencimientos están pendientes
      //  si no es así comunicar que hay que borrar asiento de cobro
      // modelo->record(indiceactual.row()).value("id_remesa").toString()
      if (model->rowCount()>0)
         {
          if (!model->record(0).value("pendiente").toBool())
             {
              QMessageBox::warning( this, tr("Domiciliación"),
                                    tr("No es posible desactivar procesada. Hay que borrar asiento de cobro"));
              ui.procesadacheckBox->disconnect(SIGNAL(stateChanged(int)));
              ui.procesadacheckBox->setChecked(true);
              connect(ui.procesadacheckBox,SIGNAL(stateChanged(int)),SLOT(checkprocesado()));
              return;
             }
         }
         ui.cuadernosgroupBox->setEnabled(true);
         ui.cobropushButton->setEnabled(true);
         ui.impagadopushButton->setEnabled(false);
        }
}


void domiciliacion::impagados()
{
    if (ui.aux_bancolineEdit->text().isEmpty() || !existesubcuenta(ui.aux_bancolineEdit->text()))
       {
        QMessageBox::warning( this, tr("Domiciliaciones"),
                              tr("ERROR: no existe la cuenta auxiliar del banco"));
        return;
       }

QModelIndex primaryKeyIndex;
primaryKeyIndex=ui.tableView->currentIndex();

if (!primaryKeyIndex.isValid()) return;
int fila=primaryKeyIndex.row();
// model->record(fila).value("revisado").toBool()
//    QString cad = model->data(model->index(fila,1),Qt::DisplayRole).toString();
//    ui.codigolabel->setText(cad);
//    ui.descriplabel->setText(descripcioncuenta(cad));
QString numvenci=model->record(fila).value("num").toString();
QString cuenta=model->record(fila).value("cta_ordenante").toString();
QString importe=model->record(fila).value("importe").toString();
QDate fecha_operacion=model->record(fila).value("fecha_operacion").toDate();
QDate fecha_vencimiento=model->record(fila).value("fecha_vencimiento").toDate();
bool pendiente=model->record(fila).value("pendiente").toBool();
bool impagado=model->record(fila).value("impagado").toBool();
QString apunte_origen=model->record(fila).value("pase_diario_operacion").toString();
QString documento_origen=basedatos::instancia()->documento_pase(apunte_origen);

if (pendiente)
  {
    QMessageBox::warning( this, tr("Marcar impagados"),
                          tr("ERROR: El registro seleccionado figura como pendiente de pago"));
    return;
  }
if (impagado)
  {
    if (QMessageBox::question(this,
        tr("Impagados/Devoluciones"),
        tr("¿ Desea eliminar la marca de <impagado> del registro marcado ?"),
        tr("&Sí"), tr("&No"),
        QString::null, 0, 1 ) )
      return ;
    // actualizamos registro de vencimentos impagado=false
    QString concepto=tr("Anulación impagado, documento origen ");
    concepto+= documento_origen;
    concepto+= " - ";
    concepto+= fecha_operacion.toString("dd-MM-yyyy");
    tabla_asientos *t = new tabla_asientos(comadecimal,decimales,usuario);
    t->fuerza_sin_vencimientos();
    t->pasadatos1(0,cuenta,concepto,"",importe,"","");
    t->pasadatos1(1,ui.aux_bancolineEdit->text(),concepto,importe,"","","");
    t->pasafechaasiento(fecha_vencimiento);
    t->chequeatotales();
    t->exec();
    delete(t);

    basedatos::instancia()->marca_vto_impagado(numvenci, false);
    refrescar();
    return;
  }

// tabla asientos con procesado por defecto
tabla_asientos *t = new tabla_asientos(comadecimal,decimales,usuario);
t->fuerzacontado();
//                 void pasadatos1( int fila, QString col0, QString col1, QString col2,
//                                   QString col3, QString col4, QString col16);
// primera fila
QString concepto=tr("Devolución, documento origen ");
concepto+= documento_origen;
concepto+= " - ";
concepto+= fecha_operacion.toString("dd-MM-yyyy");
t->pasadatos1(0,cuenta,concepto,importe,"","","");
t->pasadatos1(1,ui.aux_bancolineEdit->text(),concepto,"",importe,"","");
t->pasafechaasiento(fecha_vencimiento);
t->chequeatotales();
int resultado=t->exec();
delete(t);
if (resultado==QDialog::Accepted)
  basedatos::instancia()->marca_vto_impagado(numvenci, true);
refrescar();
}



void domiciliacion::informelatex()
{

    QString qfichero=dirtrabajo();
    qfichero.append(QDir::separator());
    qfichero=qfichero+tr("domiciliaciones.tex");
    QFile fichero(adapta(qfichero));
     if (! fichero.open( QIODevice::WriteOnly ) ) return;

     QTextStream stream( &fichero );
     stream.setCodec("UTF-8");

     stream << cabeceralatex();
     stream << margen_extra_latex();

     // stream << "\\begin{landscape}" << "\n";

     stream << "\\begin{center}" << "\n";
     stream << "{\\Large \\textbf {";
     stream << filtracad(nombreempresa()) << "}}";
     stream << "\\end{center}";

     stream << "\\begin{center}" << "\n";
     stream << "{\\textbf {";
     stream << tr("Domiciliación: ") + ui.id_domiciliacionlineEdit->text() <<
               " - " << ui.descriplineEdit->text() << "}}";
     stream << "\\end{center}";

     QString mensaje=filtracad(ui.aux_bancolineEdit->text()) + " ";
     mensaje+=filtracad(ui.descrip_bancolineEdit->text()) + " Id: ";
     mensaje+=filtracad(ui.id_domiciliacionlineEdit->text());
     stream << "\\begin{center}" << "\n";
     stream << "{\\textbf {";
     stream << mensaje << "}}";
     stream << "\\end{center}\n";

     stream << "\\begin{center}" << "\n";
     stream << tr("CCC: ") << ui.ccclineEdit->text();
     stream << " - - " << tr("IBAN: ") << ui.ibanlineEdit->text();
     stream << "\n";
     stream << "\\end{center}\n";

     // stream << "\\begin{center}" << "\n";
     stream << tr("Emisión: ") << ui.emisiondateEdit->date().toString("dd-MM-yyyy");
     stream << " - " << tr("Cargo: ") << ui.cargodateEdit->date().toString("dd-MM-yyyy");
     stream << "\n";
     // stream << "\\end{center}\n";


     // "select v.num, v.cta_ordenante, v.importe, v.fecha_operacion,"
     // "v.fecha_vencimiento, d.documento, datos.razon, datos.cif,"
     // "datos.ccc, datos.IBAN, v.pendiente, v.impagado  "

     stream << "\\begin{center}" << "\n";
     stream << "\\begin{longtable}{|r|c|p{3cm}|p{1.5cm}|c|c|r|c|c|c|c|}" << "\n";
     stream << "\\hline" << "\n";

     // --------------------------------------------------------------------------------------
     stream << "{\\tiny{" << tr("Núm.") << "}} & ";
     stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
     stream << "{\\tiny{" << tr("Razón") << "}} & ";
     stream << "{\\tiny{" << tr("Factura") << "}} & ";
     stream << "{\\tiny{" << tr("CIF") << "}} & ";
     stream << "{\\tiny{" << tr("IBAN/CCC") << "}} & ";
     stream << "{\\tiny{" << tr("Importe") << "}} & ";
     stream << "{\\tiny{" << tr("Vencim.") << "}} & ";
     stream << "{\\tiny{" << tr("Tesorería") << "}} & ";
     stream << "{\\tiny{" << tr("P.") << "}} & ";
     stream << "{\\tiny{" << tr("I.") << "}}  ";
     stream << " \\\\" << "\n";
     stream << "\\hline" << "\n";
     stream << "\\endfirsthead";
     // ------------------------------------------------------------------------------------------
     stream << "\\hline" << "\n";
     stream << "{\\tiny{" << tr("Núm.") << "}} & ";
     stream << "{\\tiny{" << tr("Cuenta") << "}} & ";
     stream << "{\\tiny{" << tr("Razón") << "}} & ";
     stream << "{\\tiny{" << tr("Factura") << "}} & ";
     stream << "{\\tiny{" << tr("CIF") << "}} & ";
     stream << "{\\tiny{" << tr("IBAN/CCC") << "}} & ";
     stream << "{\\tiny{" << tr("Importe") << "}} & ";
     stream << "{\\tiny{" << tr("Vencim.") << "}} & ";
     stream << "{\\tiny{" << tr("Tesorería") << "}} & ";
     stream << "{\\tiny{" << tr("P.") << "}} & ";
     stream << "{\\tiny{" << tr("I.") << "}} ";
     stream << " \\\\" << "\n";
     stream << "\\hline" << "\n";
     stream << "\\endhead" << "\n";
     // --------------------------------------------------------------------------------------------
     for (int veces=0; veces<model->rowCount(); veces++)
      {
         stream << "{\\tiny ";
         stream << model->record(veces).value("num").toString();
         stream << "} & {\\tiny ";
         stream << model->record(veces).value("cta_ordenante").toString();
         QSqlQuery q = basedatos::instancia()->selectTodoDatossubcuentacuenta(
                 model->record(veces).value("cta_ordenante").toString());
         QString razon,cif,ccc,iban;
         if (q.isActive())
             if (q.next())
               {
                razon=q.value(1).toString();
                cif=q.value(3).toString();
                ccc=q.value(14).toString();
                iban=q.value(36).toString();
               }
         QString documento;
         QSqlQuery q2 = basedatos::instancia()->selectAsientodocumentodiariopase(
                     model->record(veces).value("pase_diario_operacion").toString());
         if (q2.isActive())
             if (q2.next())
                documento=q2.value(1).toString();

         stream << "} & {\\tiny ";
         stream << filtracad(razon);

         stream << "} & {\\tiny ";
         stream << filtracad(documento);

         stream << "} & {\\tiny ";
         stream << filtracad(cif);
         stream << "} & {\\tiny ";
         // si hay IBAN ponerlo, si no CCC
         if (iban.isEmpty()) stream << ccc;
            else stream << iban;
         stream << "} & {\\tiny ";
         stream << filtracad(model->data(model->index(veces,3),Qt::DisplayRole).toString());
         stream << "} & {\\tiny ";

         stream << filtracad(model->data(model->index(veces,5),Qt::DisplayRole).toString());
         stream << "} & {\\tiny ";

         stream << filtracad(model->data(model->index(veces,4),Qt::DisplayRole).toString());
         stream << "} & {\\tiny ";

         if (model->record(veces).value("pendiente").toBool()) stream << "X";
         stream << "} & {\\tiny ";
         if (model->record(veces).value("impagado").toBool()) stream << "X";
         stream << "} \\\\ \n  " << "\\hline\n";
      }

     // --------------------------------------------------------------------------------------
/*
     QSqlQuery q = basedatos::instancia()->datos_domiciliacion(ui.id_domiciliacionlineEdit->text());
     // "select v.num, v.cta_ordenante, v.importe, v.fecha_operacion,"
     // "v.fecha_vencimiento, d.documento, datos.razon, datos.cif,"
     // "datos.ccc, datos.IBAN, v.pendiente, v.impagado  "

     if (q.isActive())
     while (q.next())
           {
              stream << "{\\tiny ";
              stream << q.value(0).toString();
              stream << "} & {\\tiny ";
              stream << filtracad(q.value(1).toString());
              stream << "} & {\\tiny ";
              stream << filtracad(q.value(6).toString());
              stream << "} & {\\tiny ";
              stream << filtracad(q.value(7).toString());
              stream << "} & {\\tiny ";
              stream << filtracad(q.value(8).toString());
              stream << "} & {\\tiny ";
              stream << formateanumerosep(q.value(2).toDouble(),comadecimal,decimales);
              stream << "} & {\\tiny ";
              stream << (q.value(10).toBool() ? "X" : "");
              stream << "} & {\\tiny ";
              stream << (q.value(11).toBool() ? "X" : "");
              stream << "} \\\\ \n  " << "\\hline\n";
           }
*/
     stream << "\\end{longtable}" << "\n";
     stream << "\\end{center}" << "\n";

     stream << "\\begin{center}" << "\n";
     stream << "\\begin{tabular}{|l|r|}" << "\n";
     stream << "\\hline" << "\n";
     stream << "{\\tiny ";
     stream << tr("Total domiciliación: ")<< "} & {\\tiny " << ui.sumalineEdit->text();
     stream << "} \\\\ \n  " << "\\hline\n";
     stream << "{\\tiny ";
     stream << tr("Impagados:") << "} & {\\tiny " << ui.impagadoslineEdit->text();
     stream << "} \\\\ \n  " << "\\hline\n";
     stream << "{\\tiny ";
     stream << tr("Diferencia:") << "} & {\\tiny " << ui.diferencialineEdit->text();
     stream << "} \\\\ \n  " << "\\hline\n";
     stream << "\\end{tabular}" << "\n";
     stream << "\\end{center}" << "\n";


     // stream << "\\end{landscape}\n";

     stream << "% FIN_CUERPO\n";
     stream << "\\end{document}" << "\n";

     fichero.close();

}


void domiciliacion::imprime()
{
    if (model->rowCount()==0) return;

    informelatex();

   int valor=imprimelatex2(tr("domiciliaciones"));
   if (valor==1)
       QMessageBox::warning( this, tr("Imprimir volumen de operaciones"),tr("PROBLEMAS al llamar a Latex"));
   if (valor==2)
       QMessageBox::warning( this, tr("Imprimir volumen de operaciones"),
                                tr("PROBLEMAS al llamar a 'dvips'"));
   if (valor==3)
       QMessageBox::warning( this, tr("Imprimir volumen de operaciones"),
                             tr("PROBLEMAS al llamar a ")+programa_imprimir());

}
