/* ----------------------------------------------------------------------------------
    KEME-Contabilidad ; aplicación para llevar contabilidades

    Copyright (C)  José Manuel Díez Botella

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

#include "equivalencias.h"
#include "funciones.h"
#include "basedatos.h"
#include <QProgressDialog>
#include "directorio.h"
#include <QMessageBox>
#include <QFileDialog>

equivalencias::equivalencias() : QDialog() {
    ui.setupUi(this);

 QStringList columnas;
 columnas << tr("DESCRIPCIÓN PLAN") << tr("CÓDIGO") << tr("EQUIVALENCIA");
 columnas << tr("DESCRIPCIÓN PLAN NUEVO");

 ui.tableWidget->setHorizontalHeaderLabels(columnas);

 ui.tableWidget->setColumnWidth(0,300);
 ui.tableWidget->setColumnWidth(1,100);
 ui.tableWidget->setColumnWidth(2,100);
 ui.tableWidget->setColumnWidth(3,300);

 QSqlQuery consulta = basedatos::instancia()->selectCodigodescripcionplancontablelengthauxiliarordercodigo(false);
 int fila=0;
  while (consulta.next())
      {
        if (codsubdivis(consulta.value(0).toString())) continue; // buscamos últimas ramas del plan
        ui.tableWidget->insertRow(fila);
        ui.tableWidget->setItem(fila,0,new QTableWidgetItem(consulta.value(1).toString()));
        ui.tableWidget->setItem(fila,1,new QTableWidgetItem(consulta.value(0).toString()));
        fila++;
      }
 ui.tableWidget->setCurrentCell(0,2);
 ui.tableWidget->setFocus();

 consulta = basedatos::instancia()->selectCodequivequivalenciasplan();

  connect(ui.tableWidget,SIGNAL( cellChanged ( int , int )),this,
           SLOT(valorcambiado(int,int )));

 if (consulta.isActive() && consulta.first())
   {
    if (!QMessageBox::question(this,
                tr("EDICIÓN DE EQUIVALENCIAS"),
                tr("¿ Cargar contenido guardado de equivalencias ?"),
                tr("&Sí"), tr("&No"),
                QString::null, 0, 1 ) )
        {
         do
          {
            QString cod1=consulta.value(0).toString();
            QString cod2=consulta.value(1).toString();
            for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
                 {
                  if (ui.tableWidget->item(veces,1)->text()==cod1)
                      {
                        if (codsubdivisplannuevo(cod2)) break;
                        QString qdescrip;
                        if (existecodigoplannuevo(cod2,&qdescrip))
                            {
                             if (ui.tableWidget->item(veces,2)==0)
                                 ui.tableWidget->setItem(veces,2,new QTableWidgetItem(cod2));
                             else
                               ui.tableWidget->item(veces,2)->setText(cod2);
                            }
                         break;
                       }
                  }
          } while (consulta.next());
         }
   }

 connect(ui.tableWidget,SIGNAL(  currentCellChanged ( int , int, int, int )),this,
           SLOT(posicioncambiada ( int, int)));

  connect(ui.limpiarpushButton,SIGNAL(clicked()),this,SLOT(limpiar()));

  connect(ui.exportarpushButton,SIGNAL(clicked()),this,SLOT(exportar()));

  connect(ui.importarpushButton,SIGNAL(clicked()),this,SLOT(importar()));

  connect(ui.guardarpushButton,SIGNAL(clicked()),this,SLOT(guardar()));

  connect(ui.comprobarpushButton,SIGNAL(clicked()),this,SLOT(comprueba()));

  connect(ui.asignarpushButton,SIGNAL(clicked()),this,SLOT(asignartodo()));

  connect(ui.convertirpushButton,SIGNAL(clicked()),this,SLOT(procesar()));

}


void equivalencias::posicioncambiada( int row, int col )
{
   if (col!=2) ui.tableWidget->setCurrentCell(row,2);
   ui.tableWidget->setFocus();
}


void equivalencias::valorcambiado(int fila,int columna)
{
   if (columna!=2) return;
   QString cad;
   QString cadena2;
   if (ui.tableWidget->item(fila,2)!=0)
    {
     if (ui.tableWidget->item(fila,2)->text().length()==0)
       {
        if (ui.tableWidget->item(fila,3)!=0) ui.tableWidget->item(fila,3)->setText("");
        return;
       }
     // buscamos el valor de la cuenta en el nuevo plan, si no existe
     // borrar y mostrar mensaje
     cad=ui.tableWidget->item(fila,2)->text();
     if (codsubdivisplannuevo(cad))
         {
            QMessageBox::warning( this, tr("Conversión del plan"),tr("ERROR: el código se subdivide en otros"));
            if (ui.tableWidget->item(fila,3)!=0) ui.tableWidget->item(fila,3)->setText("");
            ui.tableWidget->item(fila,2)->setText("");
            ui.tableWidget->setFocus();
            return;
         }
     QString qdescrip;
     if (existecodigoplannuevo(cad,&qdescrip))
         {
          if (ui.tableWidget->item(fila,3)==0)
             ui.tableWidget->setItem(fila,3,new QTableWidgetItem(qdescrip));
          else
             ui.tableWidget->item(fila,3)->setText(qdescrip);
         }
     else
          {
            QMessageBox::warning( this, tr("Conversión del plan"),tr("ERROR: código inexistente"));
            if (ui.tableWidget->item(fila,3)!=0) ui.tableWidget->item(fila,3)->setText("");
            ui.tableWidget->item(fila,2)->setText("");
            ui.tableWidget->setFocus();
          }
    }
}


void equivalencias::limpiar()
{
  ui.tableWidget->disconnect(SIGNAL(cellChanged(int,int)));
  for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
       {
        if (ui.tableWidget->item(veces,2)!=0)
            ui.tableWidget->item(veces,2)->setText("");
        if (ui.tableWidget->item(veces,3)!=0)
            ui.tableWidget->item(veces,3)->setText("");
       }

  connect(ui.tableWidget,SIGNAL( cellChanged ( int , int )),this,
           SLOT(valorcambiado(int,int )));

}


void equivalencias::exportar()
{
  QString nombre;

#ifdef NOMACHINE
  directorio *dir = new directorio();
  dir->pasa_directorio(dirtrabajo());
  dir->activa_pide_archivo(tr("equivalencias.txt"));
  if (dir->exec() == QDialog::Accepted)
    {
      // QMessageBox::information( this, tr("selección"),
      //                         dir->selec_actual() );
      // QMessageBox::information( this, tr("selección"),
      //                         dir->ruta_actual() );
      nombre=dir->nuevo_conruta();
      // QMessageBox::information( this, tr("selección"),nombre );
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
  filtros << tr("Archivos de texto con equivalencias (*.txt)");
  dialogofich.setNameFilters(filtros);
  dialogofich.setDirectory(dirtrabajo());
  dialogofich.setWindowTitle(tr("ARCHIVO PARA REGISTRAR EQUIVALENCIAS ENTRE PLANES"));
  QStringList fileNames;
  if (dialogofich.exec())
     {
      fileNames = dialogofich.selectedFiles();
      if (fileNames.at(0).length()>0)
          {
           nombre=fileNames.at(0);
          }
      }
#endif

    if (!nombre.isEmpty())
      {
        if (nombre.right(4)!=".txt") nombre=nombre+".txt";
        QFile fich( nombre  );
        if (fich.exists() && QMessageBox::question(this,
             tr("¿ Sobreescribir ? -- Exportar fichero"),
             tr("'%1' ya existe."
                "¿ Desea sobreescribirlo ?")
               .arg( nombre ),
             tr("&Sí"), tr("&No"),
             QString::null, 0, 1 ) )
           return ;
      } else return;



    QString pasa;
    if (eswindows()) pasa=QFile::encodeName(nombre);
       else pasa=nombre;
    QFile fichero(pasa);
    if (! fichero.open( QIODevice::WriteOnly ) ) return;
    QTextStream stream( &fichero );
    stream.setCodec("UTF-8");

    for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
       {
        if (ui.tableWidget->item(veces,2)!=0)
            if (ui.tableWidget->item(veces,2)->text().length()>0)
                {
                 stream << ui.tableWidget->item(veces,1)->text() << "\t";
                 stream << ui.tableWidget->item(veces,2)->text() << "\n";
                }
       }

  fichero.close();

}


void equivalencias::importar()
{
  QString nombre;
#ifdef NOMACHINE
  directorio *dir = new directorio();
  dir->pasa_directorio(adapta(dirtrabajo()));
  if (dir->exec() == QDialog::Accepted)
    {
      nombre=dir->ruta_actual();
    }
  delete(dir);
#else

  QFileDialog dialogofich(this);
  dialogofich.setFileMode(QFileDialog::ExistingFile);
  dialogofich.setLabelText ( QFileDialog::LookIn, tr("Directorio:") );
  dialogofich.setLabelText ( QFileDialog::FileName, tr("Archivo:") );
  dialogofich.setLabelText ( QFileDialog::FileType, tr("Tipo de archivo:") );
  dialogofich.setLabelText ( QFileDialog::Accept, tr("Aceptar") );
  dialogofich.setLabelText ( QFileDialog::Reject, tr("Cancelar") );

  QStringList filtros;
  filtros << tr("Archivos de texto (*.txt)");
  dialogofich.setNameFilters(filtros);
  dialogofich.setDirectory(dirtrabajo());
  dialogofich.setWindowTitle(tr("SELECCIÓN DE ARCHIVO PARA IMPORTAR"));
  QStringList fileNames;
  if (dialogofich.exec())
     {
      fileNames = dialogofich.selectedFiles();
      if (fileNames.at(0).length()>0)
          {
             nombre =fileNames.at(0);
          }
     }
#endif

  if (!nombre.isEmpty())
    {
      QString pasa;
      if (eswindows()) pasa=QFile::encodeName(nombre);
         else pasa=nombre;
      QFile fichero(pasa);
      if (!fichero.exists()) return;
      QTextStream stream( &fichero );
      stream.setCodec("UTF-8");
      QString linea;
      if ( fichero.open( QIODevice::ReadOnly ) )
         {
           while ( !stream.atEnd() ) {
                 linea = stream.readLine(); // linea de texto excluyendo '\n'
                 QString cod1=linea.section('\t',0,0);
                 QString cod2=linea.section('\t',1,1);
                 for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
                      {
                       if (ui.tableWidget->item(veces,1)->text()==cod1)
                           {
                            if (codsubdivisplannuevo(cod2)) break;
                            QString qdescrip;
                            if (existecodigoplannuevo(cod2,&qdescrip))
                               {
                                if (ui.tableWidget->item(veces,2)==0)
                                    ui.tableWidget->setItem(veces,2,new QTableWidgetItem(cod2));
                                else
                                    ui.tableWidget->item(veces,2)->setText(cod2);
                               }
                            break;
                           }
                      }
                }
           fichero.close();
         }
   }

}



void equivalencias::guardar()
{
    // primero borramos tabla equivalencias plan
    basedatos::instancia()->deleteEquivalenciasplan();

    for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
       {
        if (ui.tableWidget->item(veces,2)!=0)
            if (ui.tableWidget->item(veces,2)->text().length()>0)
                {
                  basedatos::instancia()->insertEquivalenciasplan( ui.tableWidget->item(veces,1)->text(),
                      ui.tableWidget->item(veces,2)->text() );
                }
       }
  QMessageBox::information( this, tr("Conversión del plan"),tr("Se han guardado las equivalencias"));
  accept();
}



bool equivalencias::convok()
{
 QStringList col1;
 QStringList col2;
 QStringList subcuentas;

    for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
         {
          QString cod1=ui.tableWidget->item(veces,1)->text();
          if (ui.tableWidget->item(veces,2)==0) continue;
             else  if (ui.tableWidget->item(veces,2)->text().length()==0) continue;
          QString cod2=ui.tableWidget->item(veces,2)->text();
          // lo primero es corregir anchos
          int ancho1=cod1.length();
          int ancho2=cod2.length();
          if (ancho1>ancho2)
              for (int veces=1;veces<=ancho1-ancho2;veces++) cod2.append('0');
          if (ancho2>ancho1)
              for (int veces=1;veces<=ancho2-ancho1;veces++) cod1.append('0');
          col1 << cod1;
          col2 << cod2;
         }


 QSqlQuery consulta = basedatos::instancia()->selectCodigoplancontablecodigoordercodigo();
 if (consulta.isActive())
 while (consulta.next())
   {
    QString codigo=consulta.value(0).toString();
    // buscamos en col1
    for (int i = 0; i < col1.size(); ++i)
         {
          if (codigo.startsWith(col1.at(i)))
              {
               // procesamos y break
               if (col2.at(i).length()==0)
                   {
                     QMessageBox::warning( this, tr("Conversión del plan"),
                               tr("ERROR: no existe código de conversión para %1").arg(col1.at(i)));
                     return false;
                   }
               QString truncado=codigo.right(codigo.length()-col1.at(i).length());
               QString destino=col2.at(i)+truncado;
               if (subcuentas.contains(destino))
                   {
                     QMessageBox::warning( this, tr("Conversión del plan"),
                               tr("ERROR: la conversión genera subcuentas duplicadas en %1").arg(destino));
                     return false;
                   }
               subcuentas << destino;
               break;
              }
           if (i==col1.size()-1)
              {
                // si llegamos al final del bucle no existe cuenta de conversión
                QMessageBox::warning( this, tr("Conversión del plan"),
                tr("ERROR: no existe código de conversión para %1").arg(codigo));
                return false;
              }
         }
   }
  // progreso.setValue(2);
  return true;
}

void equivalencias::comprueba()
{
 int bien=convok();
 
 if (bien)
     QMessageBox::information( this, tr("Conversión del plan"),
        tr("Las reglas para la conversión parecen correctas"));

}

void equivalencias::asignartodo()
{
  limpiar();
  for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
       {
        QString qdescrip;
        QString cad=ui.tableWidget->item(veces,1)->text();
        if (codsubdivisplannuevo(cad)) continue;
        if (existecodigoplannuevo(cad,&qdescrip))
         {
          if (ui.tableWidget->item(veces,2)==0)
             ui.tableWidget->setItem(veces,2,new QTableWidgetItem(cad));
          else
             ui.tableWidget->item(veces,2)->setText(cad);
         }
       }
}

void equivalencias::procesar()
{
 // hay que guardar cuentas de origen, de destino y descripciones
 if (!convok())
    {
     QMessageBox::warning( this, tr("Conversión del plan"),
        tr("ERROR: La conversión no es posible"));
     return;
    }


    if (QMessageBox::question(this,
                tr("CONVERTIR PLAN CONTABLE"),
                tr("Se va a proceder a la conversión del plan contable;\n"
                   "no olvide realizar copias de seguridad antes de proceder.\n ¿ Continuar ?"),
                tr("&Sí"), tr("&No"),
                QString::null, 0, 1 ) )
        return;


 QStringList col1;
 QStringList col2;
 QStringList subcuentasorigen,subcuentas,descripciones;

    for (int veces=0;veces<ui.tableWidget->rowCount();veces++)
         {
          QString cod1=ui.tableWidget->item(veces,1)->text();
          if (ui.tableWidget->item(veces,2)==0) continue;
             else  if (ui.tableWidget->item(veces,2)->text().length()==0) continue;
          QString cod2=ui.tableWidget->item(veces,2)->text();
          // lo primero es corregir anchos
          if (!cod_longitud_variable())
             {
              int ancho1=cod1.length();
              int ancho2=cod2.length();
              if (ancho1>ancho2)
                 for (int veces=1;veces<=ancho1-ancho2;veces++) cod2.append('0');
              if (ancho2>ancho1)
                 for (int veces=1;veces<=ancho2-ancho1;veces++) cod1.append('0');
             }
          col1 << cod1;
          col2 << cod2;
         }


 QSqlQuery consulta = basedatos::instancia()->selectCodigodescripcionplancontablelengthauxiliarordercodigo(true);

 while (consulta.next())
   {
    QString codigo=consulta.value(0).toString();
    // buscamos en col1
    for (int i = 0; i < col1.size(); ++i)
         {
          if (codigo.startsWith(col1.at(i)))
              {
               // procesamos y break
               QString truncado=codigo.right(codigo.length()-col1.at(i).length());
               QString destino=col2.at(i)+truncado;
               QString descripcion=consulta.value(1).toString();
               subcuentasorigen << codigo;
               subcuentas << destino;
               descripciones << descripcion;
               break;
              }
          // si llegamos al final del bucle no existe cuenta de conversión

         }

   }

    // basedatos::instancia()->deletePlancontable();
    // basedatos::instancia()->insertPlancontablenuevoplan();
 // insertamos nuevo plan con "9999" de prefijo en el código - mantenemos el plan antiguo

 basedatos::instancia()->insertPlancontablenuevoplan9999();

 QProgressDialog progreso(tr("Convirtiendo plan ..."), 0, 0, subcuentas.size()+1);
 progreso.setWindowModality(Qt::WindowModal);
 // progreso.setLabelText(tr("Comprobando equivalencias ..."));
 // progreso.setMaximum(2);
 // progreso.setValue(1);


 for (int veces=0; veces< subcuentas.size();veces++)
   {
     progreso.setValue(veces);
     QApplication::processEvents();
     basedatos::instancia()->insertPlancontable( "9999"+subcuentas.at(veces) , descripciones.at(veces) , true );

     cambiacuentaconta(subcuentasorigen.at(veces),"9999"+subcuentas.at(veces)); // saldossubcuenta incluido
   }

 // en teoría ya están todos los códigos con '9999'+condigo_nuevo
 // el siguiente paso: eliminamos códigos del plan contable que no empiecen por '9999'
 basedatos::instancia()->borradeplancontable_NO_9999();
 // ahora insertamos códigos en plan contable sin '9999' y subcuentas sin '9999'
 basedatos::instancia()->duplicaplancontable_NO_9999();
 for (int veces=0; veces< subcuentas.size();veces++) // ahora subcuentas
   {
     basedatos::instancia()->insertPlancontable( subcuentas.at(veces) , descripciones.at(veces) , true );
   }

 // reemplazamos todos los códigos de '9999'

 for (int veces=0; veces< subcuentas.size();veces++)
   {
     progreso.setValue(veces);
     QApplication::processEvents();
     cambiacuentaconta("9999"+subcuentas.at(veces),subcuentas.at(veces)); // saldossubcuenta incluido
   }
 // lo único que queda es eliminar del plancontable los códigos que comiencen por '9999'
 basedatos::instancia()->borradeplancontable9999();
 progreso.setValue(subcuentas.size()+1);

 accept();
}


