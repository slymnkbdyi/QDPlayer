#include "player.h"
#include "ui_player.h"
#include<QFileInfo>
#include<QDir>
#include<QDebug>
#include <QFileDialog>
#include <QDirIterator>
#include<QMessageBox>
#include <QPainter>
#include<QPen>
#include<QBrush>
#include <QTextCharFormat>
#include<QPixmap>
#include<QLabel>
#include <QColor>
#include<QRect>
#include <qmainwindow.h>
#include "plot.h"
#include <qwt_date.h>
#include <qwt_scale_widget.h>
#include <qlayout.h>
#include <qsettings.h>
#include<qtimer.h>
#include <cstdio>
#include <cstring>
#include <QVideoWidget>
#include <QtAVWidgets>
#include "wid_win.h"
#include <exception>
#include <QDesktopWidget>
using namespace QtAV;
using namespace std;

Player::Player(QWidget *parent) :
  QMainWindow(parent),
  ui(new Ui::Player)
{
  ui->setupUi(this);


  this->setStyleSheet("background-color: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 rgb(43, 75, 116), stop:1 rgb(197, 245, 254)); ");
  // this->setStyleSheet("background-color: qlineargradient(spread:reflect, x1:0.5, y1:0, x2:0, y2:0, stop:0 rgba(91, 204, 233, 100), stop:1 rgba(32, 80, 96,100)); ");
  QString tabStyle = "QTabBar {width:200;height:20;margin : 1;radius:5; color:white;font:bold;}"
                     "QTabBar::tab:!selected {width:500;height:20;margin : 1;radius:15; color:white;font:bold;font-size:10pt;}"
                     "QTabBar::tab:selected {width:500;height:20;margin : 1;radius:15; color:white;font:bold;font-size:8pt;background-color:gray}";
  ui->tabWidget->setStyleSheet(tabStyle);
  ui->calendar->setStyleSheet("selection-background-color: blue;   selection-color: white ");

  ui->Play_Btn->setStyleSheet("background-image:url(play.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->Play_Btn->setToolTip("Normal Play 1x");
  ui->Pause_btn->setStyleSheet("background-image:url(pause.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->Pause_btn->setToolTip("Pause");
  ui->Stop_Btn->setStyleSheet("background-image:url(stop.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->Stop_Btn->setToolTip("Stop");
  ui->Back_Btn->setStyleSheet("background-image:url(backward.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->Back_Btn->setToolTip("Previous");
  ui->Forward_Btn->setStyleSheet("background-image:url(forward.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->Forward_Btn->setToolTip("Next");
  ui->slow_play_button->setStyleSheet("background-image:url(slow.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->slow_play_button->setToolTip("Slow Play 0.5x");
  ui->fast_speed_btn->setStyleSheet("background-image:url(fast.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->fast_speed_btn->setToolTip("Fast Play 2x");

  ui->one_screen_btn->setStyleSheet("background-image:url(onescreen.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->one_screen_btn->setToolTip("One windows");


  ui->twoscreen_btn->setStyleSheet("background-image:url(twoscreen.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->twoscreen_btn->setToolTip("Two windows");

  ui->four_screen_btn->setStyleSheet("background-image:url(fourscreen.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->four_screen_btn->setToolTip("Four windows");

  ui->full_screen_btn->setStyleSheet("background-image:url(fullscreen.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->full_screen_btn->setToolTip("Full Screen");


  ui->snapshot_btn->setStyleSheet("background-image:url(capture.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->snapshot_btn->setToolTip("Snapshot");


  ui->video_record_btn->setStyleSheet("background-image:url(video_record.jpg) 0 0 0 0 stretch stretch ;border-width: 0px;");
  ui->video_record_btn->setToolTip("Video Record");






  //Kullanılacak nesneler oluşturuluyor
  d_plot = new Plot();
  d_plot->setMaximumHeight(120);
  ui->chart_grid_layout->addWidget(d_plot);
  /* searh computer drive directory */
  check_drive();
  QTimer *timer = new QTimer( this );
  connect( timer, SIGNAL(timeout()), this, SLOT(check_drive()) );
  timer->start( 10000 ); // 2 seconds single-shot timer
  cx = new wid_win;

  ui->renderer_layout->addWidget(cx,0,1);

  d_plot->set_player(this);
  cx->set_player(this);


  connect(ui->snapshot_btn, SIGNAL(clicked()), SLOT(capture()));
  connect(cx->m_player->videoCapture(0), SIGNAL(saved(QString)), SLOT(onCaptureSaved(QString)));
  connect(cx->m_player->videoCapture(0), SIGNAL(failed()), SLOT(onCaptureError()));


  connect(cx->m_player->videoCapture(1), SIGNAL(saved(QString)), SLOT(onCaptureSaved(QString)));
  connect(cx->m_player->videoCapture(1), SIGNAL(failed()), SLOT(onCaptureError()));


  connect(cx->m_player->videoCapture(2), SIGNAL(saved(QString)), SLOT(onCaptureSaved(QString)));
  connect(cx->m_player->videoCapture(2), SIGNAL(failed()), SLOT(onCaptureError()));


  connect(cx->m_player->videoCapture(3), SIGNAL(saved(QString)), SLOT(onCaptureSaved(QString)));
  connect(cx->m_player->videoCapture(3), SIGNAL(failed()), SLOT(onCaptureError()));
}

Player::~Player()
{
  delete ui;
}


void Player::reSize()
{

  qDebug()<< " widget height : " << cx->m_renderer1->widget()->height();
  qDebug() << "widget width   :" << cx->m_renderer1->widget()->width();
  qDebug()<< " frame  height  : " << cx->m_renderer1->videoFrameSize().height();
  qDebug() << " frame width : "<<   cx->m_renderer1 ->videoFrameSize().width();
  qDebug()<< " rect size :"<<            cx->m_renderer1->videoRect().height();
  qDebug()<<" rect size width : "<<cx->m_renderer1->videoRect().width();
  qDebug()<<"----------------------------------------- \n\n\n";



  if(cx->m_player->isPlaying())
    {
      //      cx->m_renderer1->widget()->resize(cx->m_renderer1->videoRect().width(),cx->m_renderer1->videoRect().height());
      //      cx->m_renderer2->widget()->resize(cx->m_renderer2->videoRect().width(),cx->m_renderer2->videoRect().height());
      //      cx->m_renderer3->widget()->resize(cx->m_renderer3->videoRect().width(),cx->m_renderer3->videoRect().height());
      //      cx->m_renderer4->widget()->resize(cx->m_renderer4->videoRect().width(),cx->m_renderer4->videoRect().height());

    }
  qDebug()<< " widget height : " << cx->m_renderer1->widget()->height();
  qDebug() << "widget width   :" << cx->m_renderer1->widget()->width();
  qDebug()<< " frame  height  : " << cx->m_renderer1->videoFrameSize().height();
  qDebug() << " frame width : "<<   cx->m_renderer1 ->videoFrameSize().width();
  qDebug()<< " rect size :"<<            cx->m_renderer1->videoRect().height();
  qDebug()<<" rect size width : "<<cx->m_renderer1->videoRect().width();

}

void Player::mousePressEvent(QMouseEvent *event)
{


}

void Player::check_drive()
{
  drive_list.clear();
  foreach( QFileInfo drive, QDir::drives())
    {
      drive_list<<drive.absolutePath();
    }
  if(ui->drive_cbx->count() != drive_list.count())
    {
      ui->drive_cbx->clear();
      for(int i=0;i<drive_list.count();i++)
        {
          ui->drive_cbx->addItem(drive_list[i]);
        }
    }

}

void Player::on_select_folder_btn_clicked()
{

  fileName.clear();
  fileName = QFileDialog::getExistingDirectory(this,"Select");
  if(fileName.isEmpty())
    {
      message_box.setText("Please select folder ");
      message_box.show();
      return;
    }
  else
    {
      search_file_and_directory(fileName);
    }
}


QList<QDate>date_list;
QStringList date_path_list;
QString new_path;
QString rdb_file;
QString basket_file;
QString last_basket_file;
void Player::search_file_and_directory(QString path)
{

  QString rmdb;

  QDirIterator it(path, QDirIterator::Subdirectories);
  while (it.hasNext())
    {
      rmdb=it.next();
      if(rmdb.contains("/rdb"))
        {

          qDebug()<<"found  : "<<rmdb;
          new_path=rmdb;
          break;
        }
    }
  QString rmd_member;
  QDate date;
  QDate max_date;
  bool ok(false);
  if(new_path !=0)
    {
      QDirIterator it(new_path, QDirIterator::Subdirectories);
      while (it.hasNext())
        {
          rmd_member=it.next();
          rmd_member=it.fileName();
          date_path_list.append(it.filePath());
          date.setDate(rmd_member.mid(0,4).toInt(&ok),rmd_member.mid(4,2).toInt(&ok),rmd_member.mid(6,2).toInt(&ok));
          date_list.append(date);
        }
      QTextCharFormat day_light;
      if(date_list.size()>2)
        {
          max_date=date_list[2];
          for(int i=2;i<date_list.size();i++)
            {
              //day_light.setBackground(Qt::cyan);
              QFont serifFont("Times", 10, QFont::Bold);
              day_light.setFont(serifFont);
              day_light.setForeground(QColor(Qt::blue));
              ui->calendar->setDateTextFormat(date_list[i],day_light);
              if(max_date<date_list[i])
                {
                  max_date=date_list[i];
                }
            }
          d_plot->applySettings(max_date);
          find_path(max_date);

          day_light.setBackground(QColor(Qt::blue));
          ui->calendar->setSelectedDate(max_date);
        }
      else
        {
          message_box.setText("Not found record at  " +new_path );
          message_box.show();
        }
    }
  else
    {
      message_box.setText("Not found RMDB folder ");
      message_box.show();
    }
}

QString select_date_string;

void Player::on_calendar_clicked(const QDate &date)
{
  if(date_list.contains(date))
    {

      d_plot->applySettings(date);
      find_path(date);
      qDebug()<<"enter ";
    }
  else
    {
      qDebug()<<"not found selected date ";

    }
}
void Player::find_path(QDate date)
{
  QString find_date=date.toString("yyyyMMdd");
  for(int i=0;i<date_path_list.size();i++)
    {
      if(date_path_list[i].contains(find_date))
        {
          read_RdbFile(date_path_list[i], date);
          break;
        }
    }
}

QStringList exist_record_list;
void Player::read_RdbFile(QString file_path,QDate date)
{
  exist_record_list.clear();
  qDebug()<<"path: "<< file_path;
  qDebug() << "*n\n\n Start read rdb file ";
  d_plot->first_paint(convertdate(date,0),convertdate(date,1439),1);
  d_plot->first_paint(convertdate(date,0),convertdate(date,1439),2);
  d_plot->first_paint(convertdate(date,0),convertdate(date,1439),3);
  d_plot->first_paint(convertdate(date,0),convertdate(date,1439),4);
  QFile file(file_path);
  if(!file.open(QIODevice::ReadOnly))
    {
      return;
    }
  qDebug()<< "size t1 :"<< sizeof(t1);
  blob = file.readAll();
  QString blob_str = blob.toHex();
  qDebug()<<"reading file   :"<<  file_path;
  qDebug()<<file.size();
  int current_value=0;
  int index ;
  bool ok(false);
  QString bid_str;
  QString idx_str;
  for(int i=0;i<MAX_CHANNEL;i++)
    {
      int start_value=0;
      bool control=false;
      int   last_vaule=0;
      for(int j=0;j<MINUTES_OF_DAY;j++)
        {
          index = 1440*2*8 + 1440*16*i+16*(j);
          current_value=blob[i*1440+j];
          bid_str=blob_str.mid(index+6,2)+blob_str.mid(index+4,2)+blob_str.mid(index+2,2)+blob_str.mid(index+0,2);
          idx_str=blob_str.mid(index+14,2)+blob_str.mid(index+12,2)+blob_str.mid(index+10,2)+blob_str.mid(index+8,2);
          int bid_int =bid_str.toInt(&ok,16);
          int idx_int=idx_str.toInt(&ok,16);
          t1.RdbFile[j][i].bid=bid_int;
          t1.RdbFile[j][i].idx_pos=idx_int;
          //    qDebug()<< "index :" <<   index << "camera       :  "<<i <<   "minutes    :  "<< j    <<   "basket_id : "<<t1.RdbFile[j][i].bid<<     "idx_pos  : "<< t1.RdbFile[j][i].idx_pos;
          if(current_value==last_vaule)
            {
              if(control==false)
                {
                  qDebug()<<"---------------------üst---------------------------";
                  qDebug() <<"last value    :   "<< last_vaule  << "current value : "     <<current_value;
                  qDebug()<< "i   :      "<< i << "      j:    "<< j;
                  qDebug()<<"---------------------üst---------------------------";
                  start_value=j;
                  control=true;
                }
            }
          else
            {
              if(control==true)
                {
                  if(last_vaule==0)
                    {
                      qDebug()<<"---------------------0 icin---------------------------";
                      qDebug() <<"last value    :   "<< last_vaule  << "current value : "     <<current_value;
                      qDebug()<< "i   :      "<< i << "      j:    "<< j;
                      qDebug()<<"---------------------0 icin---------------------------";
                    }
                  if(last_vaule==1)
                    {
                      qDebug()<<"--------------------1 icin---------------------------";
                      qDebug() <<"last value    :   "<< last_vaule  << "current value : "     <<current_value;
                      qDebug()<< "i   :      "<< i << "      j:    "<< j;
                      qDebug()<<"---------------------1 icin---------------------------";
                      for(;last_vaule<=j;last_vaule++)
                        {
                          if(!exist_record_list.contains(QString::number(last_vaule)))
                            {
                              exist_record_list += QString::number(last_vaule,10);
                            }

                        }
                      d_plot->paint(convertdate(date,start_value),convertdate(date,j),1,i+1);
                    }
                  if(last_vaule==2)
                    {
                      qDebug()<<current_value;
                      qDebug()<< "i   :      "<< i << "     j:    "<< j;
                      d_plot->paint(convertdate(date,start_value),convertdate(date,j),2,i+1);
                    }
                  if(last_vaule==3)
                    {
                      d_plot->paint(convertdate(date,start_value),convertdate(date,j),3,i+1);
                      qDebug()<<current_value;
                    }
                  last_vaule=current_value;
                  control=false;
                  start_value=0;
                }
              else
                {
                  last_vaule=current_value;
                }
            }
        }
    }
  d_plot->Stick1(convertdate(date,1439));
  file.close();
}

void Player::on_search_btn_clicked()
{
  search_file_and_directory(ui->drive_cbx->currentText());
}


QDateTime Player::convertdate(QDate date,int value)
{
  QString time_str = QDateTime::fromTime_t(value*60).toUTC().toString("hh:mm");
  QTime time = QTime::fromString(time_str,"hh:mm");
  QDateTime return_date=QDateTime(date,time,Qt::UTC);
  return return_date;
}
void Player::find_basket_id_and_pos(int  value)
{
  int find_bkt=0;
  if(exist_record_list.contains(QString::number(value,10)))
    {
      for(int ch=0;ch<MAX_CHANNEL;ch++)
        {
          if(t1.RdbFile[value][ch].bid !=0  && t1.RdbFile[value][ch].idx_pos!=0)
            {
              find_bkt=1;
              QString index_file_name ;
              int path_ind=new_path.indexOf("rdb");
              index_file_name=QString::number( t1.RdbFile[value][ch].bid);
              while(index_file_name.size()<8)
                {
                  index_file_name="0"+index_file_name;
                }
              rdb_file=new_path.mid(0, path_ind)+index_file_name+".idx";
              basket_file=new_path.mid(0, path_ind)+index_file_name+".bkt";
              cx->m_player->setFile(basket_file);
              read_Index_File(new_path.mid(0, path_ind)+index_file_name+".idx",t1.RdbFile[value][ch].idx_pos);
            }
        }

    }
  if(find_bkt==0)
    {
      if(basket_file.size() !=0)
        {
          cx->m_player->stop();
        }

    }


}


int Player::sec_to_string_buf(time_t t, char *buf, int buf_len)
{
  struct tm *p;
  if (!(p = localtime(&t)))
    {
      qDebug()<<" date conversion error ";
    }
  if (!strftime(buf, buf_len, "%Y-%m-%d %H:%M:%S", p))
    {
      qDebug()<< " date conversion error 2 ";
    }
  return 0;
}

int Player::pts_to_string_buf(int64_t pts, char *buf, int buf_len)
{
  time_t t = pts/1000000;
  return sec_to_string_buf(t, buf, buf_len);
}
int first_seek=0;
void Player::read_Index_File(QString file_path,unsigned int fpos )
{

  qDebug()<< " read index "<< file_path;
  FILE *idx;
  char* cstr;
  string fname = file_path.toStdString();
  cstr = new char [fname.size()+1];
  strcpy( cstr, fname.c_str() );
  idx = fopen(cstr,"rb");
  if(!idx)
    {
      qDebug()<<"not open idx file ";
    }
  else
    {
      T_INDEX_HDR ihd;
      if(fread((unsigned char *)&ihd, 1, sizeof(T_INDEX_HDR), idx) < sizeof(T_INDEX_HDR))
        {
          qDebug()<<"failed index header file ";
        }
      else
        {
          if(QString::number(ihd.id, 16) =="f0000001")
            {
              int index_count=0;
              T_INDEX_DATA idd;
              int64_t t1[MAX_CHANNEL],t2[MAX_CHANNEL];
              for(int ch=0;ch<MAX_CHANNEL;ch++)
                {
                  t1[ch] = 0;
                  t2[ch] = 0;
                }
              fseek(idx,fpos,SEEK_SET);
              if(fread((unsigned char *)&idd, 1, sizeof(T_INDEX_DATA), idx) && index_count<=ihd.count )
                {
                  if(first_seek==0)
                    {
                      if(QString::number(idd.id, 16) =="f0000002")
                        {
                          int64_t pos, pts;
                          pts = idd.ts.sec * (int64_t)1000000 + idd.ts.usec;
                          pos = idd.fpos;
                          char t1_buf[32];
                          pts_to_string_buf(pts, t1_buf, sizeof(t1_buf));
                          qDebug()<< QString::number(idd.id, 16) << "T_TS :  " << t1_buf << " idd fpos :"<<idd.fpos << " idd ch :"<< idd.ch<< " s_type : "<< idd.s_type<< "event :"<< idd.event  << " width :   "<< idd.width << " height : "<< idd.height << " cap mode :"<< idd.capMode;
                          if(cx->m_player->isPlaying())
                            {
                              cx->m_player->stop();
                            }
                          cx->m_player->setFile(basket_file);
                          QVariantHash opt;
                          opt[QString::fromLatin1("jump_to_pos")] = QString::fromLatin1(QByteArray::number(idd.fpos));
                          qDebug()<< " idd fpos :  "<<  idd.fpos   << " byte array      : "<< QByteArray::number(idd.fpos)<< " width height  "<< idd.width   << idd.height;
                          cx->m_player->setOptionsForFormat(opt);
                          cx->m_player->play();
                          first_seek==1;
                        }
                      else
                        {
                          qDebug() << "index data id   is not correct :" << idd.id;
                        }
                    }

                }
            }
          else
            {
              qDebug()<< " index heard id is not correct ";
            }
        }
    }
}




void Player::on_Play_Btn_clicked()
{
  if (cx->m_player->isPaused())
    {
      cx->m_player->pause(false);
    }
  else
    {
      cx->m_player->setSpeed(1);
      cx->m_player->play();
      //  reSize();
    }

}

void Player::on_Stop_Btn_clicked()
{
  if(cx->m_player->isPlaying())
    {
      cx->m_player->stop();

    }
}

void Player::on_Back_Btn_clicked()
{

  cx->m_player->seekBackward();
}

void Player::on_Forward_Btn_clicked()
{
  cx->m_player->seekForward();
}


void Player::on_fast_speed_btn_clicked()
{
  cx->m_player->setSpeed(2);
}

void Player::on_slow_play_button_clicked()
{
  cx->m_player->setSpeed(0.5);
}

void Player::on_ch1_radio_btn_clicked()
{
  for(int i=0;i<=3;i++)
    {
      AudioOutput* ao = cx->m_player->audio(i);
      if(ao)
        {
          if(i==0)
            {
              ao->setMute(false);
            }
          else
            {
              ao->setMute(true);
            }
        }
    }
}

void Player::on_ch2_radio_btn_clicked()
{
  for(int i=0;i<=3;i++)
    {
      AudioOutput* ao = cx->m_player->audio(i);
      if(ao)
        {
          if(i==1)
            {
              ao->setMute(false);
            }
          else
            {
              ao->setMute(true);
            }
        }
    }
}

void Player::on_ch3_radio_btn_clicked()
{
  for(int i=0;i<=3;i++)
    {
      AudioOutput* ao = cx->m_player->audio(i);
      if(ao)
        {
          if(i==2)
            {
              ao->setMute(false);
            }
          else
            {
              ao->setMute(true);
            }
        }
    }
}

void Player::on_ch3_radio_btn_2_clicked()
{
  for(int i=0;i<=3;i++)
    {
      AudioOutput* ao = cx->m_player->audio(i);
      if(ao)
        {
          if(i==3)
            {
              ao->setMute(false);
            }
          else
            {
              ao->setMute(true);
            }
        }
    }
}



void Player::on_Pause_btn_clicked()
{

  if (cx->m_player->isPlaying()) {

      cx->m_player->pause(true);
    }
  qDebug()<< " frame size : " << cx->m_renderer1->videoFrameSize();


}

void Player::on_mute_chckbx_stateChanged(int arg1)
{
  if(ui->mute_chckbx->isChecked())
    {

      for(int i=0;i<=3;i++)
        {
          AudioOutput* ao = cx->m_player->audio(i);
          if(ao)
            {
              ao->setMute(false);
            }
        }

    }
  else
    {
      for(int i=0;i<=3;i++)
        {
          AudioOutput* ao = cx->m_player->audio(i);
          if(ao)
            {
              ao->setMute(true);
            }
          ui->ch1_radio_btn->setChecked(false);
          ui->ch2_radio_btn->setChecked(false);
          ui->ch3_radio_btn->setChecked(false);
          ui->ch3_radio_btn_2->setChecked(false);

        }
    }
}

void Player::on_one_screen_btn_clicked()
{
  cx->clicked_state=1;
  cx->m_renderer1->widget()->setVisible(false);
  cx->m_renderer2->widget()->setVisible(false);
  cx->m_renderer3->widget()->setVisible(false);
  cx->m_renderer4->widget()->setVisible(false);
  cx->m_renderer1->widget()->setVisible(true);

  reSize();
  cx->screen_mode=1;
  update();
}


void Player::on_twoscreen_btn_clicked()
{
  cx->clicked_state=1;
  cx->m_renderer1->widget()->setVisible(false);
  cx->m_renderer2->widget()->setVisible(false);
  cx->m_renderer3->widget()->setVisible(false);
  cx->m_renderer4->widget()->setVisible(false);
  cx->m_renderer1->widget()->setVisible(true);
  cx->m_renderer2->widget()->setVisible(true);
  reSize();
  cx->screen_mode=2;

  update();
}

void Player::on_four_screen_btn_clicked()
{
  cx->clicked_state=1;
  cx->m_renderer1->widget()->setVisible(false);
  cx->m_renderer2->widget()->setVisible(false);
  cx->m_renderer3->widget()->setVisible(false);
  cx->m_renderer4->widget()->setVisible(false);
  cx->m_renderer1->widget()->setVisible(true);
  cx->m_renderer2->widget()->setVisible(true);
  cx->m_renderer3->widget()->setVisible(true);
  cx->m_renderer4->widget()->setVisible(true);
  reSize();
  cx->screen_mode=4;
  update();

}


void Player::on_full_screen_btn_clicked()
{


  int screen_number = QApplication::desktop()->screenNumber(this);
  QDesktopWidget *desktop = QApplication::desktop();
  if(cx->full_btn_state==0)
    {
      cx->setParent(0);
      cx->show();

      if(1==screen_number)
        {
          cx->windowHandle()->setScreen(qApp->screens()[1]);
          cx->showFullScreen();

        }
      if(0==screen_number)
        {
          cx->showFullScreen();
        }

      cx->full_btn_state=1;
      this->hide();
    }
}

void Player::exitFullScreen()
{
  qDebug()<<cx->full_btn_state;
  cx->full_btn_state=0;

  cx->setParent(this);
  ui->renderer_layout->addWidget(cx);
  this->show();
}


void Player::capture()
{

  qDebug()<< " captured ";
    cx->m_player->videoCapture(cx->a)->setCaptureDir("/tmp");
  cx->m_player->videoCapture(cx->a)->capture();

}

void Player::onCaptureSaved(const QString &path)
{
  QDateTime d1(QDateTime::currentDateTime());
  qDebug()<<d1;
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                         "/Capture"+d1.toString("_yyyyMMdd_hhmmss") +".png",
                         tr("Images (*.png)"));


  if(QFile::copy(path,fileName))
    {
      qDebug()<<"success";
       ui->capture_path->setText("\nCapture Path :\n  " + fileName+"\n" );
    }
  else
    {
      qDebug()<<"not succes copy ";
    }


}

void Player::onCaptureError()
{
  QMessageBox::warning(0, QString::fromLatin1("QtAV video capture"), tr("Failed to capture video frame"));
}

