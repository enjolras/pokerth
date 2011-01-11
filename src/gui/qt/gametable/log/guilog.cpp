/***************************************************************************
 *   Copyright (C) 2006 by FThauer FHammer   *
 *   f.thauer@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "guilog.h"

#include "gametableimpl.h"
#include <handinterface.h>
#include <session.h>
#include <game.h>
#include <game_defs.h>
#include "gametablestylereader.h"

using namespace std;

guiLog::guiLog(gameTableImpl* w, ConfigFile *c) : myW(w), myConfig(c), myLogDir(0), myHtmlLogFile(0), mySqliteLogDb(0)
{

    myW->setGuiLog(this);
    myStyle = myW->getMyGameTableStyle();

    myAppDataPath = QString::fromUtf8(myConfig->readConfigString("AppDataDir").c_str());

    connect(this, SIGNAL(signalLogPlayerActionMsg(QString, int, int, int)), this, SLOT(logPlayerActionMsg(QString, int, int, int)));
    connect(this, SIGNAL(signalLogNewGameHandMsg(int, int)), this, SLOT(logNewGameHandMsg(int, int)));
    connect(this, SIGNAL(signalLogNewBlindsSetsMsg(int, int, QString, QString)), this, SLOT(logNewBlindsSetsMsg(int, int, QString, QString)));
    connect(this, SIGNAL(signalLogPlayerWinsMsg(QString, int, bool)), this, SLOT(logPlayerWinsMsg(QString, int, bool)));
    connect(this, SIGNAL(signalLogPlayerSitsOut(QString)), this, SLOT(logPlayerSitsOut(QString)));
    connect(this, SIGNAL(signalLogDealBoardCardsMsg(int, int, int, int, int, int)), this, SLOT(logDealBoardCardsMsg(int, int, int, int, int, int)));
    connect(this, SIGNAL(signalLogFlipHoleCardsMsg(QString, int, int, int, int, QString)), this, SLOT(logFlipHoleCardsMsg(QString, int, int, int, int, QString)));
    connect(this, SIGNAL(signalLogPlayerLeftMsg(QString, int)), this, SLOT(logPlayerLeftMsg(QString, int)));
    connect(this, SIGNAL(signalLogNewGameAdminMsg(QString)), this, SLOT(logNewGameAdminMsg(QString)));
    connect(this, SIGNAL(signalLogPlayerWinGame(QString, int)), this, SLOT(logPlayerWinGame(QString, int)));
    connect(this, SIGNAL(signalFlushLogAtGame(int)), this, SLOT(flushLogAtGame(int)));
    connect(this, SIGNAL(signalFlushLogAtHand()), this, SLOT(flushLogAtHand()));


    logFileStreamString = "";
    lastGameID = 0;

    if(myConfig->readConfigInt("LogOnOff")) {
        //if write logfiles is enabled

        QDir logDir(QString::fromUtf8(myConfig->readConfigString("LogDir").c_str()));

        if(myConfig->readConfigString("LogDir") != "" && logDir.exists()) {

            int i;

            if(HTML_LOG) {

                myLogDir = new QDir(QString::fromUtf8(myConfig->readConfigString("LogDir").c_str()));
                QDateTime currentTime = QDateTime::currentDateTime();
                myHtmlLogFile = new QFile(myLogDir->absolutePath()+"/pokerth-log-"+currentTime.toString("yyyy-MM-dd_hh.mm.ss")+".html");

                //Logo-Pixmap extrahieren
                QPixmap logoChipPixmapFile(":/gfx/logoChip3D.png");
                logoChipPixmapFile.save(myLogDir->absolutePath()+"/logo.png");

//                myW->textBrowser_Log->append(myHtmlLogFile->fileName());

                // erstelle html-Datei
                myHtmlLogFile->open( QIODevice::WriteOnly );
                QTextStream stream( myHtmlLogFile );
                stream << "<html>\n";
                stream << "<head>\n";
                stream << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\">";
                stream << "</head>\n";
                stream << "<body style=\"font-size:smaller\">\n";
                stream << "<img src='logo.png'>\n";
                stream << QString("<h3><b>Log-File for PokerTH %1 Session started on ").arg(POKERTH_BETA_RELEASE_STRING)+QDate::currentDate().toString("yyyy-MM-dd")+" at "+QTime::currentTime().toString("hh:mm:ss")+"</b></h3>\n";
                myHtmlLogFile->close();

            }

            // delete old log files
            int daysUntilWaste = myConfig->readConfigInt("LogStoreDuration");

            QStringList filters("pokerth-log*");
            QStringList logFileList = myLogDir->entryList(filters, QDir::Files);

            for(i=0; i<logFileList.count(); i++) {

//                cout << logFileList.at(i).toStdString() << endl;

                QString dateString = logFileList.at(i);
                dateString.remove("pokerth-log-");
                dateString.remove(10,14);

                QDate dateOfFile(QDate::fromString(dateString, Qt::ISODate));
                QDate today(QDate::currentDate());

//                cout << dateOfFile.daysTo(today) << endl;

                if (dateOfFile.daysTo(today) > daysUntilWaste) {

    //                cout << QString::QString(myLogDir->absolutePath()+"/"+logFileList.at(i)).toStdString() << endl;
                    QFile fileToDelete(myLogDir->absolutePath()+"/"+logFileList.at(i));
                    fileToDelete.remove();
                }

            }

        } else {
            cout << "Log directory doesn't exist. Cannot create log files";
        }
    }
}

guiLog::~guiLog()
{
    delete myLogDir;
    delete myHtmlLogFile;

}

void guiLog::logPlayerActionMsg(QString msg, int playerID, int action, int setValue) {

	switch (action) {

		case 1: msg += " folds.";
		break;
		case 2: msg += " checks.";
		break;
		case 3: msg += " calls ";
		break;
		case 4: msg += " bets ";
		break;
		case 5: msg += " bets ";
		break;
		case 6: msg += " is all in with ";
		break;
		default: msg += "ERROR";
	}
	
	if (action >= 3) { msg += "$"+QString::number(setValue,10)+"."; }
	
    myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+msg+"</span>");

	if(myConfig->readConfigInt("LogOnOff")) {
        //if write logfiles is enabled

        if(HTML_LOG) {

            logFileStreamString += msg+"</br>\n";

            if(myConfig->readConfigInt("LogInterval") == 0) {
                writeLogFileStream(logFileStreamString);
                logFileStreamString = "";
            }

        }

        if(SQLITE_LOG) {

//            if(mySqliteLogDb->isOpen()) {

//                QSqlQuery query(*mySqliteLogDb);
//                QString sql;

//                sql = "INSERT INTO Action (HandID,GameID,BeRo,Player,Action,Amount) VALUES (";
//                sql += QString::number(myW->getSession()->getCurrentGame()->getCurrentHandID(),10);
//                sql += "," + QString::number(myW->getSession()->getCurrentGame()->getMyGameID(),10);
//                sql += "," + QString::number(myW->getSession()->getCurrentGame()->getCurrentHand()->getCurrentBeRo()->getMyBeRoID(),10);
//                sql += "," + QString::number(playerID,10);
//                sql += "," + QString::number(action,10);
//                sql += "," + QString::number(setValue,10);
//                sql += ")";

//                if(myConfig->readConfigInt("LogInterval") == 0) {
//                    if(!query.exec(sql)) {
//                        QMessageBox::critical(0, tr("ERROR"),query.lastError().text().toUtf8().data(), QMessageBox::Cancel);
//                    }
//                }


//            } else {
//                QMessageBox::critical(0, tr("ERROR"),mySqliteLogDb->lastError().text().toUtf8().data(), QMessageBox::Cancel);
//            }

        }

	}
}

void guiLog::logNewGameHandMsg(int gameID, int handID) {

	PlayerListConstIterator it_c;
	boost::shared_ptr<HandInterface> currentHand = myW->getSession()->getCurrentGame()->getCurrentHand();

        PlayerList activePlayerList = currentHand->getActivePlayerList();

    myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getChatLogTextColor()+"; font-size:large; font-weight:bold\">## Game: "+QString::number(gameID,10)+" | Hand: "+QString::number(handID,10)+" ##</span>");

	if(myConfig->readConfigInt("LogOnOff")) {
        //if write logfiles is enabled

        if(HTML_LOG) {

            logFileStreamString += "<table><tr><td width=\"600\" align=\"center\"><hr noshade size=\"3\"><b>Game: "+QString::number(gameID,10)+" | Hand: "+QString::number(handID,10)+"</b></td><td></td></tr></table>";
            logFileStreamString += "BLIND LEVEL: $"+QString::number(currentHand->getSmallBlind())+" / $"+QString::number(currentHand->getSmallBlind()*2)+"</br>";

            //print cash only for active players
            for(it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); it_c++) {

                logFileStreamString += "Seat " + QString::number((*it_c)->getMyID()+1,10) + ": <b>" +  QString::fromUtf8((*it_c)->getMyName().c_str()) + "</b> ($" + QString::number((*it_c)->getMyCash()+(*it_c)->getMySet(),10)+")</br>";

            }

        }

        if(SQLITE_LOG) {

            string sql;
            int i;
            char *errmsg;

            if( mySqliteLogDb != 0 ) {

                if(handID == 1) {
                    // start of a game -> insert game data

                    sql = "INSERT INTO Game (";
                        sql += "GameID";
                        sql += ",Startmoney";
                        sql += ",StartSb";
                        sql += ",DealerPos";
                        for(i=1; i<=MAX_NUMBER_OF_PLAYERS; i++) {
                            sql += ",Seat_" + boost::lexical_cast<std::string>(i);
                        }
                    sql += ") VALUES (";
                        sql += boost::lexical_cast<string>(gameID);
                        sql += "," + boost::lexical_cast<string>(myW->getSession()->getCurrentGame()->getStartCash());
                        sql += "," + boost::lexical_cast<string>(myW->getSession()->getCurrentGame()->getStartSmallBlind());
                        sql += "," + boost::lexical_cast<string>(myW->getSession()->getCurrentGame()->getDealerPosition());
                        PlayerList seatList = currentHand->getSeatsList();
                        for(it_c = seatList->begin();it_c!=seatList->end();it_c++) {
                            if((*it_c)->getMyActiveStatus()) {
                                sql += ",\"" + (*it_c)->getMyName() +"\"";
                            } else {
                                sql += ",NULL";
                            }
                        }
                        sql += ")";
                    if(sqlite3_exec(mySqliteLogDb, sql.data(), 0, 0, &errmsg) != SQLITE_OK) {
                        cout << "Error in statement: " << sql.data() << "[" << errmsg << "]." << endl;
                    }

                }

                // insert hand data
                sql = "INSERT INTO Hand (";
                    sql += "HandID";
                    sql += ",GameID";
                    sql += ",Dealer_Seat";
                    sql += ",Sb_Amount";
                    sql += ",Sb_Seat";
                    sql += ",Bb_Amount";
                    sql += ",Bb_Seat";
                sql += ") VALUES (";

//                // hand data
//                sql = "INSERT INTO Hand (HandID,GameID,Sb_Amount,Bb_Amount";
//                for(it_c=currentHand->getActivePlayerList()->begin(); it_c!=currentHand->getActivePlayerList()->end(); it_c++) {
//                    sql += ",Seat_" + QString::number((*it_c)->getMyID()+1,10) + "_Cash";
//                }
////                for(i=1;i<=MAX_NUMBER_OF_PLAYERS; i++) {
////                    sql += ",Seat_" + QString("%1").arg(i) + "_Cash";
////                }
//                sql += ") VALUES (";
//                sql += QString::number(handID,10);
//                sql += "," + QString::number(gameID,10);
//                sql += "," + QString::number(currentHand->getSmallBlind(),10);
//                sql += "," + QString::number(currentHand->getSmallBlind()*2,10);
//                for(it_c=currentHand->getActivePlayerList()->begin(); it_c!=currentHand->getActivePlayerList()->end(); it_c++) {
//                    sql += "," + QString::number((*it_c)->getMyCash()+(*it_c)->getMySet(),10);
//                }
////                for(it_c = currentHand->getSeatsList()->begin();it_c!=currentHand->getSeatsList()->end();it_c++) {
////                    if((*it_c)->getMyActiveStatus()) {
////                        sql += "," + QString::number((*it_c)->getMyCash()+(*it_c)->getMySet(),10);
////                    } else {
////                        sql += ",NULL";
////                    }
////                }
//                sql += ")";

            }


//            if(mySqliteLogDb->isOpen()) {

//                QSqlQuery query(*mySqliteLogDb);
//                QString sql;

//                // beginning of a game
//                if(handID == 1) {
//                    sql = "INSERT INTO Game (GameID,Startmoney,StartSb,DealerPos";
//                    for(it_c=currentHand->getActivePlayerList()->begin(); it_c!=currentHand->getActivePlayerList()->end(); it_c++) {
//                        sql += ",Seat_" + QString::number((*it_c)->getMyID()+1,10);
//                    }
////                    for(i=1;i<=MAX_NUMBER_OF_PLAYERS; i++) {
////                        sql += ",Seat_" + QString::number(i,10);
////                    }
//                    sql += ") VALUES (";
//                    sql += QString::number(gameID,10) + ",";
//                    sql += QString::number(myW->getSession()->getCurrentGame()->getStartCash(),10) + ",";
//                    sql += QString::number(myW->getSession()->getCurrentGame()->getStartSmallBlind(),10) + ",";
//                    sql += QString::number(myW->getSession()->getCurrentGame()->getDealerPosition(),10);
//                    for(it_c=currentHand->getActivePlayerList()->begin(); it_c!=currentHand->getActivePlayerList()->end(); it_c++) {
//                        sql += ",'" + QString::fromUtf8((*it_c)->getMyName().c_str()) +"'";
//                    }
////                    for(it_c = currentHand->getSeatsList()->begin();it_c!=currentHand->getSeatsList()->end();it_c++) {
////                        if((*it_c)->getMyActiveStatus()) {
////                            sql += ",'" + QString::fromUtf8((*it_c)->getMyName().c_str()) +"'";
////                        } else {
////                            sql += ",NULL";
////                        }
////                    }
//                    sql += ")";
//                    if(!query.exec(sql)) {
//                        QMessageBox::critical(0, tr("ERROR"),query.lastError().text().toUtf8().data(), QMessageBox::Cancel);
//                    }
//                }

//                // beginning of a hand
//                sql = "INSERT INTO Hand (HandID,GameID,Sb_Amount,Bb_Amount";
//                for(it_c=currentHand->getActivePlayerList()->begin(); it_c!=currentHand->getActivePlayerList()->end(); it_c++) {
//                    sql += ",Seat_" + QString::number((*it_c)->getMyID()+1,10) + "_Cash";
//                }
////                for(i=1;i<=MAX_NUMBER_OF_PLAYERS; i++) {
////                    sql += ",Seat_" + QString("%1").arg(i) + "_Cash";
////                }
//                sql += ") VALUES (";
//                sql += QString::number(handID,10);
//                sql += "," + QString::number(gameID,10);
//                sql += "," + QString::number(currentHand->getSmallBlind(),10);
//                sql += "," + QString::number(currentHand->getSmallBlind()*2,10);
//                for(it_c=currentHand->getActivePlayerList()->begin(); it_c!=currentHand->getActivePlayerList()->end(); it_c++) {
//                    sql += "," + QString::number((*it_c)->getMyCash()+(*it_c)->getMySet(),10);
//                }
////                for(it_c = currentHand->getSeatsList()->begin();it_c!=currentHand->getSeatsList()->end();it_c++) {
////                    if((*it_c)->getMyActiveStatus()) {
////                        sql += "," + QString::number((*it_c)->getMyCash()+(*it_c)->getMySet(),10);
////                    } else {
////                        sql += ",NULL";
////                    }
////                }
//                sql += ")";

//                if(!query.exec(sql)) {
//                    QMessageBox::critical(0, tr("ERROR"),query.lastError().text().toUtf8().data(), QMessageBox::Cancel);
//                }

//            } else {
//                QMessageBox::critical(0, tr("ERROR"),mySqliteLogDb->lastError().text().toUtf8().data(), QMessageBox::Cancel);
//            }


        }

	}

}

void guiLog::logNewBlindsSetsMsg(int sbSet, int bbSet, QString sbName, QString bbName) {

    // log blinds
    myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+sbName+" posts small blind ($"+QString::number(sbSet,10)+")</span>");
    myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+bbName+" posts big blind ($"+QString::number(bbSet,10)+")</span>");

    if(myConfig->readConfigInt("LogOnOff")) {
        //if write logfiles is enabled

        if(HTML_LOG) {

            logFileStreamString += "BLINDS: ";

            logFileStreamString += sbName+" ($"+QString::number(sbSet,10)+"), ";
            logFileStreamString += bbName+" ($"+QString::number(bbSet,10)+")";

            PlayerListConstIterator it_c;
            boost::shared_ptr<HandInterface> currentHand = myW->getSession()->getCurrentGame()->getCurrentHand();
            PlayerList activePlayerList = currentHand->getActivePlayerList();

            for(it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); it_c++) {

                if(activePlayerList->size() > 2) {
                    if((*it_c)->getMyButton() == BUTTON_DEALER) {

                        logFileStreamString += "</br>" + QString::fromUtf8((*it_c)->getMyName().c_str()) + " starts as dealer.";
                        break;
                    }
                } else {
                    if((*it_c)->getMyButton() == BUTTON_SMALL_BLIND) {

                        logFileStreamString += "</br>" + QString::fromUtf8((*it_c)->getMyName().c_str()) + " starts as dealer.";
                        break;
                    }
                }
            }

            logFileStreamString += "</br></br><b>PREFLOP</b>";
            logFileStreamString += "</br>\n";

        }

        if(SQLITE_LOG) {



        }

    }

}

void guiLog::logPlayerWinsMsg(QString playerName, int pot, bool main) {

	if(main) {
		myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getLogWinnerMainPotColor()+";\">"+playerName+" wins $"+QString::number(pot,10)+"</span>");
	} else {
		myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getLogWinnerSidePotColor()+";\">"+playerName+" wins $"+QString::number(pot,10)+" (side pot)</span>");
	}
	
	if(myConfig->readConfigInt("LogOnOff")) {
	//if write logfiles is enabled

		logFileStreamString += "</br><i>"+playerName+" wins $"+QString::number(pot,10);
		if(!main) {
			logFileStreamString += " (side pot)";
		}
		logFileStreamString += "</i>\n";

		if(myConfig->readConfigInt("LogInterval") == 0) {	
			writeLogFileStream(logFileStreamString);
			logFileStreamString = "";
		}
	}
}

void guiLog::logPlayerSitsOut(QString playerName) {

	myW->textBrowser_Log->append("<i><span style=\"color:#"+myStyle->getLogPlayerSitsOutColor()+";\">"+playerName+" sits out</span></i>");
	
	if(myConfig->readConfigInt("LogOnOff")) {

		logFileStreamString += "</br><i><span style=\"font-size:smaller;\">"+playerName+" sits out</span></i>\n";

		if(myConfig->readConfigInt("LogInterval") == 0) {	
			writeLogFileStream(logFileStreamString);
			logFileStreamString = "";
		}
	}
}

void guiLog::logDealBoardCardsMsg(int roundID, int card1, int card2, int card3, int card4, int card5) {
	
	QString round;
	
	switch (roundID) {

		case 1: round = "Flop";
                myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getLogPlayerSitsOutColor()+";\">--- "+round+" --- "+"["+translateCardCode(card1).at(0)+translateCardCode(card1).at(1)+","+translateCardCode(card2).at(0)+translateCardCode(card2).at(1)+","+translateCardCode(card3).at(0)+translateCardCode(card3).at(1)+"]</span>");
		break;
		case 2: round = "Turn";
                myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getLogPlayerSitsOutColor()+";\">--- "+round+" --- "+"["+translateCardCode(card1).at(0)+translateCardCode(card1).at(1)+","+translateCardCode(card2).at(0)+translateCardCode(card2).at(1)+","+translateCardCode(card3).at(0)+translateCardCode(card3).at(1)+","+translateCardCode(card4).at(0)+translateCardCode(card4).at(1)+"]</span>");
		break;
		case 3: round = "River";
                myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getLogPlayerSitsOutColor()+";\">--- "+round+" --- "+"["+translateCardCode(card1).at(0)+translateCardCode(card1).at(1)+","+translateCardCode(card2).at(0)+translateCardCode(card2).at(1)+","+translateCardCode(card3).at(0)+translateCardCode(card3).at(1)+","+translateCardCode(card4).at(0)+translateCardCode(card4).at(1)+","+translateCardCode(card5).at(0)+translateCardCode(card5).at(1)+"]</span>");
		break;
		default: round = "ERROR";
	}

	if(myConfig->readConfigInt("LogOnOff")) {
        //if write logfiles is enabled

        if(HTML_LOG) {

            switch (roundID) {

                    case 1: round = "Flop";
                    logFileStreamString += "</br><b>"+round.toUpper()+"</b> [board cards <b>"+translateCardCode(card1).at(0)+"</b>"+translateCardCode(card1).at(1)+",<b>"+translateCardCode(card2).at(0)+"</b>"+translateCardCode(card2).at(1)+",<b>"+translateCardCode(card3).at(0)+"</b>"+translateCardCode(card3).at(1)+"]"+"</br>\n";
                    break;
                    case 2: round = "Turn";
                    logFileStreamString += "</br><b>"+round.toUpper()+"</b> [board cards <b>"+translateCardCode(card1).at(0)+"</b>"+translateCardCode(card1).at(1)+",<b>"+translateCardCode(card2).at(0)+"</b>"+translateCardCode(card2).at(1)+",<b>"+translateCardCode(card3).at(0)+"</b>"+translateCardCode(card3).at(1)+",<b>"+translateCardCode(card4).at(0)+"</b>"+translateCardCode(card4).at(1)+"]"+"</br>\n";
                    break;
                    case 3: round = "River";
                    logFileStreamString += "</br><b>"+round.toUpper()+"</b> [board cards <b>"+translateCardCode(card1).at(0)+"</b>"+translateCardCode(card1).at(1)+",<b>"+translateCardCode(card2).at(0)+"</b>"+translateCardCode(card2).at(1)+",<b>"+translateCardCode(card3).at(0)+"</b>"+translateCardCode(card3).at(1)+",<b>"+translateCardCode(card4).at(0)+"</b>"+translateCardCode(card4).at(1)+",<b>"+translateCardCode(card5).at(0)+"</b>"+translateCardCode(card5).at(1)+"]"+"</br>\n";
                    break;
                    default: round = "ERROR";
            }

            if(myConfig->readConfigInt("LogInterval") == 0) {
                writeLogFileStream(logFileStreamString);
                logFileStreamString = "";
            }

        }

        if(SQLITE_LOG) {

//            if(mySqliteLogDb->isOpen()) {

//                QSqlQuery query(*mySqliteLogDb);
//                QString sql;
//                sql = "UPDATE Hand SET ";

//                switch (roundID) {
//                    case 1: round = "Flop";
//                        sql += "BoardCard_1 = " + QString::number(card1,10) + ",";
//                        sql += "BoardCard_2 = " + QString::number(card2,10) + ",";
//                        sql += "BoardCard_3 = " + QString::number(card3,10);
//                    break;
//                    case 2: round = "Turn";
//                        sql += "BoardCard_4 = " + QString::number(card4,10);
//                    break;
//                    case 3: round = "River";
//                        sql += "BoardCard_5 = " + QString::number(card5,10);
//                    break;
//                    default: round = "ERROR";
//                }
//                sql += " WHERE ";
//                sql += "HandID = " + QString::number(myW->getSession()->getCurrentGame()->getCurrentHandID(),10) + " AND ";
//                sql += "GameID = " + QString::number(myW->getSession()->getCurrentGame()->getMyGameID(),10);


//                if(myConfig->readConfigInt("LogInterval") == 0) {
//                    if(roundID <= 3) {
//                        if(!query.exec(sql)) {
//                            QMessageBox::critical(0, tr("ERROR"),query.lastError().text().toUtf8().data(), QMessageBox::Cancel);
//                        }

//                    }
//                }

//            } else {
//                QMessageBox::critical(0, tr("ERROR"),mySqliteLogDb->lastError().text().toUtf8().data(), QMessageBox::Cancel);
//            }

        }

	}
}

void guiLog::logFlipHoleCardsMsg(QString playerName, int playerID, int card1, int card2, int cardsValueInt, QString showHas) {

    QString tempHandName;

	if (cardsValueInt != -1) {
	
        tempHandName = determineHandName(cardsValueInt);

        myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+playerName+" "+showHas+" \""+tempHandName+"\"</span>");

	}
	else {

        myW->textBrowser_Log->append("<span style=\"color:#"+myStyle->getChatLogTextColor()+";\">"+playerName+" "+showHas+" ["+translateCardCode(card1).at(0)+translateCardCode(card1).at(1)+","+translateCardCode(card2).at(0)+translateCardCode(card2).at(1)+"]</span>");
		
	}

    if(myConfig->readConfigInt("LogOnOff")) {
        //if write logfiles is enabled

        if(HTML_LOG) {

            if (cardsValueInt != -1) {

                tempHandName = determineHandName(cardsValueInt);

                logFileStreamString += playerName+" "+showHas+" [ <b>"+translateCardCode(card1).at(0)+"</b>"+translateCardCode(card1).at(1)+",<b>"+translateCardCode(card2).at(0)+"</b>"+translateCardCode(card2).at(1)+"] - "+tempHandName+"</br>\n";

                if(myConfig->readConfigInt("LogInterval") == 0) {
                    writeLogFileStream(logFileStreamString);
                    logFileStreamString = "";
                }

            } else {

                logFileStreamString += playerName+" "+showHas+" [<b>"+translateCardCode(card1).at(0)+"</b>"+translateCardCode(card1).at(1)+",<b>"+translateCardCode(card2).at(0)+"</b>"+translateCardCode(card2).at(1)+"]"+"</br>\n";
                if(myConfig->readConfigInt("LogInterval") == 0) {
                    writeLogFileStream(logFileStreamString);
                    logFileStreamString = "";
                }
            }
        }

        if(SQLITE_LOG) {

//            if(mySqliteLogDb->isOpen()) {

//                QString sql = "UPDATE Hand SET ";
//                sql += "Seat_" + QString::number(playerID+1,10) + "_Card_1 = " + QString::number(card1,10) + ",";
//                sql += "Seat_" + QString::number(playerID+1,10) + "_Card_2 = " + QString::number(card2,10);
//                sql += " WHERE ";
//                sql += "HandID = " + QString::number(myW->getSession()->getCurrentGame()->getCurrentHandID(),10) + " AND ";
//                sql += "GameID = " + QString::number(myW->getSession()->getCurrentGame()->getMyGameID(),10);

//                QSqlQuery query(*mySqliteLogDb);
//                if(!query.exec(sql)) {
//                    QMessageBox::critical(0, tr("ERROR"),query.lastError().text().toUtf8().data(), QMessageBox::Cancel);
//                }

//            } else {
//                QMessageBox::critical(0, tr("ERROR"),mySqliteLogDb->lastError().text().toUtf8().data(), QMessageBox::Cancel);
//            }

        }

    }
	
}

void guiLog::logPlayerLeftMsg(QString playerName, int wasKicked) {

	QString action;
	if(wasKicked) action = "was kicked from";
	else action = "has left";

        myW->textBrowser_Log->append( "<span style=\"color:#"+myStyle->getChatLogTextColor()+";\"><i>"+playerName+" "+action+" the game!</i></span>");
	
	if(myConfig->readConfigInt("LogOnOff")) {
	
		logFileStreamString += "<i>"+playerName+" "+action+" the game!</i><br>\n";

		if(myConfig->readConfigInt("LogInterval") == 0) {	
			writeLogFileStream(logFileStreamString);
			logFileStreamString = "";
		}
	}
}

void guiLog::logNewGameAdminMsg(QString playerName) {

	myW->textBrowser_Log->append( "<i><span style=\"color:#"+myStyle->getLogNewGameAdminColor()+";\">"+playerName+" is game admin now!</span></i>");
	
	if(myConfig->readConfigInt("LogOnOff")) {
	
		logFileStreamString += "<i>"+playerName+" is game admin now!</i><br>\n";

		if(myConfig->readConfigInt("LogInterval") == 0) {	
			writeLogFileStream(logFileStreamString);
			logFileStreamString = "";
		}
	}
}

void guiLog::logPlayerWinGame(QString playerName, int gameID) {

	myW->textBrowser_Log->append( "<i><b>"+playerName+" wins game " + QString::number(gameID,10)  +"!</i></b><br>");
	
	if(myConfig->readConfigInt("LogOnOff")) {
	
		logFileStreamString += "</br></br><i><b>"+playerName+" wins game " + QString::number(gameID,10)  +"!</i></b></br>\n";

		if(myConfig->readConfigInt("LogInterval") == 0) {	
			writeLogFileStream(logFileStreamString);
			logFileStreamString = "";
		}
	}

}

QStringList guiLog::translateCardCode(int cardCode) {

	int value = cardCode%13;
	int color = cardCode/13;
	
	QStringList cardString;
		
	switch (value) {
	
		case 0: cardString << "2";
		break;
		case 1: cardString << "3";
		break;
		case 2: cardString << "4";
		break;
		case 3: cardString << "5";
		break;
		case 4: cardString << "6";
		break;
		case 5: cardString << "7";
		break;
		case 6: cardString << "8";
		break;
		case 7: cardString << "9";
		break;
		case 8: cardString << "T";
		break;
		case 9: cardString << "J";
		break;
		case 10: cardString << "Q";
		break;
		case 11: cardString << "K";
		break;
		case 12: cardString << "A";
		break;
		default: cardString << "ERROR";
	}

	switch (color) {
	
		case 0: cardString << "d";
		break;
		case 1: cardString << "h";
		break;
		case 2: cardString << "s";
		break;
		case 3: cardString << "c";
		break;
		default: cardString << "ERROR";
	}
	
	return cardString;
}

QStringList guiLog::translateCardsValueCode(int cardsValueCode) {

	QStringList cardString;

	//erste Ziffer : Blattname
	int firstPart = cardsValueCode/100000000;
	//zweite und dritte Ziffer : Kicker, highest Card, usw.
	int secondPart = cardsValueCode/1000000 - firstPart*100;
	//vierte und fünfte Ziffer
	int thirdPart = cardsValueCode/10000 - firstPart*10000 - secondPart*100;
	// usw
	int fourthPart = cardsValueCode/100 - firstPart*1000000 - secondPart*10000 - thirdPart*100;
	//
	int fifthPart = cardsValueCode - firstPart*100000000 - secondPart*1000000 - thirdPart*10000 - fourthPart*100;
	// fuer highest Card
	int fifthPartA = cardsValueCode/10 - firstPart*10000000 - secondPart*100000 - thirdPart*1000 - fourthPart*10;
	int fifthPartB = cardsValueCode - firstPart*100000000 - secondPart*1000000 - thirdPart*10000 - fourthPart*100 - fifthPartA*10;


	switch (firstPart) {

		// Royal Flush
		case 9: cardString << "Royal Flush";
		break;
		// Straight Flush
		case 8: {
			cardString << "Straight Flush, ";
			switch(secondPart) {
				case 11: cardString << "King high";
				break;
				case 10: cardString << "Queen high";
				break;
				case 9: cardString << "Jack high";
				break;
				case 8: cardString << "Ten high";
				break;
				case 7: cardString << "Nine high";
				break;
				case 6: cardString << "Eight high";
				break;
				case 5: cardString << "Seven high";
				break;
				case 4: cardString << "Six high";
				break;
				case 3: cardString << "Five high";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Four of a Kind
		case 7: {
			cardString << "Four of a Kind, ";
			switch(secondPart) {
				case 12: cardString << "Aces";
				break;
				case 11: cardString << "Kings";
				break;
				case 10: cardString << "Queens";
				break;
				case 9: cardString << "Jacks";
				break;
				case 8: cardString << "Tens";
				break;
				case 7: cardString << "Nines";
				break;
				case 6: cardString << "Eights";
				break;
				case 5: cardString << "Sevens";
				break;
				case 4: cardString << "Sixes";
				break;
				case 3: cardString << "Fives";
				break;
				case 2: cardString << "Fours";
				break;
				case 1: cardString << "Threes";
				break;
				case 0: cardString << "Deuces";
				break;
				default: cardString << "ERROR";
			}
			// Kicker
			switch(thirdPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Full House
		case 6: {
			cardString << "Full House, ";
			//Drilling
			switch(secondPart) {
				case 12: cardString << "Aces full of ";
				break;
				case 11: cardString << "Kings full of ";
				break;
				case 10: cardString << "Queens full of ";
				break;
				case 9: cardString << "Jacks full of ";
				break;
				case 8: cardString << "Tens full of ";
				break;
				case 7: cardString << "Nines full of ";
				break;
				case 6: cardString << "Eights full of ";
				break;
				case 5: cardString << "Sevens full of ";
				break;
				case 4: cardString << "Sixes full of ";
				break;
				case 3: cardString << "Fives full of ";
				break;
				case 2: cardString << "Fours full of ";
				break;
				case 1: cardString << "Threes full of ";
				break;
				case 0: cardString << "Deuces full of ";
				break;
				default: cardString << "ERROR";
			}
			//Pärchen
			switch(thirdPart) {
				case 12: cardString << "Aces";
				break;
				case 11: cardString << "Kings";
				break;
				case 10: cardString << "Queens";
				break;
				case 9: cardString << "Jacks";
				break;
				case 8: cardString << "Tens";
				break;
				case 7: cardString << "Nines";
				break;
				case 6: cardString << "Eights";
				break;
				case 5: cardString << "Sevens";
				break;
				case 4: cardString << "Sixes";
				break;
				case 3: cardString << "Fives";
				break;
				case 2: cardString << "Fours";
				break;
				case 1: cardString << "Threes";
				break;
				case 0: cardString << "Deuces";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Flush
		case 5: {
			cardString << "Flush, ";
			//highest Card
			switch(secondPart) {
				case 12: cardString << "Ace high";
				break;
				case 11: cardString << "King high";
				break;
				case 10: cardString << "Queen high";
				break;
				case 9: cardString << "Jack high";
				break;
				case 8: cardString << "Ten high";
				break;
				case 7: cardString << "Nine high";
				break;
				case 6: cardString << "Eight high";
				break;
				case 5: cardString << "Seven high";
				break;
				case 4: cardString << "Six high";
				break;
				default: cardString << "ERROR";
			}
			// Kicker 1
			switch(thirdPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 2
			switch(fourthPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 3
			switch(fifthPartA) {
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 4
			switch(fifthPartB) {
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Straight
		case 4: {
			cardString << "Straight, ";
			switch(secondPart) {
				case 12: cardString << "Ace high";
				break;
				case 11: cardString << "King high";
				break;
				case 10: cardString << "Queen high";
				break;
				case 9: cardString << "Jack high";
				break;
				case 8: cardString << "Ten high";
				break;
				case 7: cardString << "Nine high";
				break;
				case 6: cardString << "Eight high";
				break;
				case 5: cardString << "Seven high";
				break;
				case 4: cardString << "Six high";
				break;
				case 3: cardString << "Five high";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Three of a Kind
		case 3: {
			cardString << "Three of a Kind, ";
			switch(secondPart) {
				case 12: cardString << "Aces";
				break;
				case 11: cardString << "Kings";
				break;
				case 10: cardString << "Queens";
				break;
				case 9: cardString << "Jacks";
				break;
				case 8: cardString << "Tens";
				break;
				case 7: cardString << "Nines";
				break;
				case 6: cardString << "Eights";
				break;
				case 5: cardString << "Sevens";
				break;
				case 4: cardString << "Sixes";
				break;
				case 3: cardString << "Fives";
				break;
				case 2: cardString << "Fours";
				break;
				case 1: cardString << "Threes";
				break;
				case 0: cardString << "Deuces";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 1
			switch(thirdPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 2
			switch(fourthPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Two Pairs
		case 2: {
			cardString << "Two Pairs, ";
			// erster Pair
			switch(secondPart) {
				case 12: cardString << "Aces and ";
				break;
				case 11: cardString << "Kings and ";
				break;
				case 10: cardString << "Queens and ";
				break;
				case 9: cardString << "Jacks and ";
				break;
				case 8: cardString << "Tens and ";
				break;
				case 7: cardString << "Nines and ";
				break;
				case 6: cardString << "Eights and ";
				break;
				case 5: cardString << "Sevens and ";
				break;
				case 4: cardString << "Sixes and ";
				break;
				case 3: cardString << "Fives and ";
				break;
				case 2: cardString << "Fours and ";
				break;
				case 1: cardString << "Threes and ";
				break;
				case 0: cardString << "Deuces and ";
				break;
				default: cardString << "ERROR";
			}
			// zweiter Pair
			switch(thirdPart) {
				case 12: cardString << "Aces";
				break;
				case 11: cardString << "Kings";
				break;
				case 10: cardString << "Queens";
				break;
				case 9: cardString << "Jacks";
				break;
				case 8: cardString << "Tens";
				break;
				case 7: cardString << "Nines";
				break;
				case 6: cardString << "Eights";
				break;
				case 5: cardString << "Sevens";
				break;
				case 4: cardString << "Sixes";
				break;
				case 3: cardString << "Fives";
				break;
				case 2: cardString << "Fours";
				break;
				case 1: cardString << "Threes";
				break;
				case 0: cardString << "Deuces";
				break;
				default: cardString << "ERROR";
			}
			//Kicker
			switch(fourthPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// One Pair
		case 1: {
			cardString << "One Pair, ";
			// Pair
			switch(secondPart) {
				case 12: cardString << "Aces";
				break;
				case 11: cardString << "Kings";
				break;
				case 10: cardString << "Queens";
				break;
				case 9: cardString << "Jacks";
				break;
				case 8: cardString << "Tens";
				break;
				case 7: cardString << "Nines";
				break;
				case 6: cardString << "Eights";
				break;
				case 5: cardString << "Sevens";
				break;
				case 4: cardString << "Sixes";
				break;
				case 3: cardString << "Fives";
				break;
				case 2: cardString << "Fours";
				break;
				case 1: cardString << "Threes";
				break;
				case 0: cardString << "Deuces";
				break;
				default: cardString << "ERROR";
			}
			// Kicker 1
			switch(thirdPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 2
			switch(fourthPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 3
			switch(fifthPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		// Highest Card
		case 0:  {
			cardString << "High Card, ";
			// Kicker 1
			switch(secondPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuces";
				break;
				default: cardString << "ERROR";
			}
			// Kicker 2
			switch(thirdPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 3
			switch(fourthPart) {
				case 12: cardString << "Ace";
				break;
				case 11: cardString << "King";
				break;
				case 10: cardString << "Queen";
				break;
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 4
			switch(fifthPartA) {
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
			//Kicker 5
			switch(fifthPartB) {
				case 9: cardString << "Jack";
				break;
				case 8: cardString << "Ten";
				break;
				case 7: cardString << "Nine";
				break;
				case 6: cardString << "Eight";
				break;
				case 5: cardString << "Seven";
				break;
				case 4: cardString << "Six";
				break;
				case 3: cardString << "Five";
				break;
				case 2: cardString << "Four";
				break;
				case 1: cardString << "Three";
				break;
				case 0: cardString << "Deuce";
				break;
				default: cardString << "ERROR";
			}
		}
		break;
		default: cardString << "ERROR";

	}

	return cardString;

}

QString guiLog::determineHandName(int myCardsValueInt) {

	list<int> shownCardsValueInt;
	list<int> sameHandCardsValueInt;
	bool different = false;
	bool equal = false;
	boost::shared_ptr<HandInterface> currentHand = myW->getSession()->getCurrentGame()->getCurrentHand();
	PlayerListConstIterator it_c;
        PlayerList activePlayerList = currentHand->getActivePlayerList();

	// collect cardsValueInt of all players who will show their cards
        for(it_c=activePlayerList->begin(); it_c!=activePlayerList->end(); it_c++) {

		if( (*it_c)->getMyAction() != PLAYER_ACTION_FOLD) {
			shownCardsValueInt.push_back( (*it_c)->getMyCardsValueInt());
		}
	}

	// erase own cardsValueInt
	list<int>::iterator it;
	for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
		if((*it) == myCardsValueInt) {
			shownCardsValueInt.erase(it);
			break;
		}
	}

	QString handName;

	switch(myCardsValueInt/100000000) {
		// Royal Flush
		case 9: {
			handName = translateCardsValueCode(myCardsValueInt).at(0);
		}
		break;
		// Straight Flush
		case 8: {
			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);
		}
		break;
		// Four of a kind
		case 7: {

			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);

			// same hand detection
			for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
				if(((*it)/1000000) == (myCardsValueInt/1000000)) {
					sameHandCardsValueInt.push_back(*it);
				}
			}

			// 1.same hands existing
			if(!(sameHandCardsValueInt.empty())) {
				// first kicker?
				for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
					if(((*it)/10000) == (myCardsValueInt/10000)) {
						equal = true;
						it++;
					} else {
						different = true;
						it = sameHandCardsValueInt.erase(it);
					}
				}
				if(different) {
					handName += ", fifth card " + translateCardsValueCode(myCardsValueInt).at(2);
				}

			}

		}
		break;
		// Full House
		case 6: {
			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1)+translateCardsValueCode(myCardsValueInt).at(2);
		}
		break;
		// Flush
		case 5: {

			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);

			// same hand detection
			for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
				if(((*it)/1000000) == (myCardsValueInt/1000000)) {
					sameHandCardsValueInt.push_back(*it);
				}
			}

			// 1.same hands existing
			if(!(sameHandCardsValueInt.empty())) {
				// first kicker?
				for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
					if(((*it)/10000) == (myCardsValueInt/10000)) {
						equal = true;
						it++;
					} else {
						different = true;
						it = sameHandCardsValueInt.erase(it);
					}
				}
				if(different) {
					handName += ", second card " + translateCardsValueCode(myCardsValueInt).at(2);
				}
				// 2.there are still same hands
				if(equal) {
					different = false;
					equal = false;
					// second kicker?
					for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
						if(((*it)/100) == (myCardsValueInt/100)) {
							equal = true;
							it++;
						} else {
							different = true;
							it = sameHandCardsValueInt.erase(it);
						}
					}
					if(different) {
						handName += ", third card " + translateCardsValueCode(myCardsValueInt).at(3);
					}
					// 3.there are still same hands
					if(equal) {
						different = false;
						equal = false;
						// third kicker?
						for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
							if(((*it)/10) == (myCardsValueInt/10)) {
								equal = true;
								it++;
							} else {
								different = true;
								it = sameHandCardsValueInt.erase(it);
							}
						}
						if(different) {
							handName += ", fourth card " + translateCardsValueCode(myCardsValueInt).at(4);
						}
						// 4.there are still same hands
						if(equal) {
							different = false;
							equal = false;
							// third kicker?
							for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
								if((*it) == myCardsValueInt) {
									equal = true;
									it++;
								} else {
									different = true;
									it = sameHandCardsValueInt.erase(it);
								}
							}
							if(different) {
								handName += ", fifth card " + translateCardsValueCode(myCardsValueInt).at(5);
							}
						}
					}
				}
			}

		}
		break;
		// Straight
		case 4: {
			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);
		}
		break;
		// Three of a kind
		case 3: {
			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);

			// same hand detection
			for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
				if(((*it)/1000000) == (myCardsValueInt/1000000)) {
					sameHandCardsValueInt.push_back(*it);
				}
			}

			// 1.same hands existing
			if(!(sameHandCardsValueInt.empty())) {
				// first kicker?
				for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
					if(((*it)/10000) == (myCardsValueInt/10000)) {
						equal = true;
						it++;
					} else {
						different = true;
						it = sameHandCardsValueInt.erase(it);
					}
				}
				if(different) {
					handName += ", fourth card " + translateCardsValueCode(myCardsValueInt).at(2);
				}
				// 2.there are still same hands
				if(equal) {
					different = false;
					equal = false;
					// second kicker?
					for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
						if(((*it)/100) == (myCardsValueInt/100)) {
							equal = true;
							it++;
						} else {
							different = true;
							it = sameHandCardsValueInt.erase(it);
						}
					}
					if(different) {
						handName += ", fifth card " + translateCardsValueCode(myCardsValueInt).at(3);
					}
				}
			}

		}
		break;
		// Two Pairs
		case 2: {

			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1)+translateCardsValueCode(myCardsValueInt).at(2);

			// same hand detection
			for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
				if(((*it)/10000) == (myCardsValueInt/10000)) {
					sameHandCardsValueInt.push_back(*it);
				}
			}

			// 1.same hands existing
			if(!(sameHandCardsValueInt.empty())) {
				// first kicker?
				for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
					if(((*it)/100) == (myCardsValueInt/100)) {
						equal = true;
						it++;
					} else {
						different = true;
						it = sameHandCardsValueInt.erase(it);
					}
				}
				if(different) {
					handName += ", fifth card " + translateCardsValueCode(myCardsValueInt).at(3);
				}

			}

		}
		break;
		// Pair
		case 1: {

			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);

			// same hand detection
			for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
				if(((*it)/1000000) == (myCardsValueInt/1000000)) {
					sameHandCardsValueInt.push_back(*it);
				}
			}

			// 1.same hands existing
			if(!(sameHandCardsValueInt.empty())) {
				// first kicker?
				for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
					if(((*it)/10000) == (myCardsValueInt/10000)) {
						equal = true;
						it++;
					} else {
						different = true;
						it = sameHandCardsValueInt.erase(it);
					}
				}
				if(different) {
					handName += ", third card " + translateCardsValueCode(myCardsValueInt).at(2);
				}
				// 2.there are still same hands
				if(equal) {
					different = false;
					equal = false;
					// second kicker?
					for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
						if(((*it)/100) == (myCardsValueInt/100)) {
							equal = true;
							it++;
						} else {
							different = true;
							it = sameHandCardsValueInt.erase(it);
						}
					}
					if(different) {
						handName += ", fourth card " + translateCardsValueCode(myCardsValueInt).at(3);
					}
					// 3.there are still same hands
					if(equal) {
						different = false;
						equal = false;
						// third kicker?
						for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
							if((*it) == myCardsValueInt) {
								equal = true;
								it++;
							} else {
								different = true;
								it = sameHandCardsValueInt.erase(it);
							}
						}
						if(different) {
							handName += ", fifth card " + translateCardsValueCode(myCardsValueInt).at(4);
						}
					}
				}
			}

		}
		break;
		// highestCard
		case 0: {

			handName = translateCardsValueCode(myCardsValueInt).at(0)+translateCardsValueCode(myCardsValueInt).at(1);

			// same hand detection
			for(it = shownCardsValueInt.begin(); it != shownCardsValueInt.end(); it++) {
				if(((*it)/1000000) == (myCardsValueInt/1000000)) {
					sameHandCardsValueInt.push_back(*it);
				}
			}

			// 1.same hands existing
			if(!(sameHandCardsValueInt.empty())) {
				// first kicker?
				for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
					if(((*it)/10000) == (myCardsValueInt/10000)) {
						equal = true;
						it++;
					} else {
						different = true;
						it = sameHandCardsValueInt.erase(it);
					}
				}
				if(different) {
					handName += ", second card " + translateCardsValueCode(myCardsValueInt).at(2);
				}
				// 2.there are still same hands
				if(equal) {
					different = false;
					equal = false;
					// second kicker?
					for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
						if(((*it)/100) == (myCardsValueInt/100)) {
							equal = true;
							it++;
						} else {
							different = true;
							it = sameHandCardsValueInt.erase(it);
						}
					}
					if(different) {
						handName += ", third card " + translateCardsValueCode(myCardsValueInt).at(3);
					}
					// 3.there are still same hands
					if(equal) {
						different = false;
						equal = false;
						// third kicker?
						for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
							if(((*it)/10) == (myCardsValueInt/10)) {
								equal = true;
								it++;
							} else {
								different = true;
								it = sameHandCardsValueInt.erase(it);
							}
						}
						if(different) {
							handName += ", fourth card " + translateCardsValueCode(myCardsValueInt).at(4);
						}
						// 4.there are still same hands
						if(equal) {
							different = false;
							equal = false;
							// third kicker?
							for(it = sameHandCardsValueInt.begin(); it != sameHandCardsValueInt.end(); ) {
								if((*it) == myCardsValueInt) {
									equal = true;
									it++;
								} else {
									different = true;
									it = sameHandCardsValueInt.erase(it);
								}
							}
							if(different) {
								handName += ", fifth card " + translateCardsValueCode(myCardsValueInt).at(5);
							}
						}
					}
				}
			}

		}
		break;
		default: {}
	}

	return handName;

}

void guiLog::writeLogFileStream(QString streamString) {

        if(myHtmlLogFile) {
                if(myHtmlLogFile->open( QIODevice::ReadWrite )) {
                        QTextStream stream( myHtmlLogFile );
			stream.readAll();
			stream << streamString;
                        myHtmlLogFile->close();
		}
		else { cout << "Could not open log-file to write log-messages!" << endl; }
	}
	else { cout << "Could not find log-file to write log-messages!" << endl; }
}

void guiLog::flushLogAtHand() {

	if(myConfig->readConfigInt("LogOnOff")) {
		if(myConfig->readConfigInt("LogInterval") < 2) {
	// 	write for log after every action and after every hand
			writeLogFileStream(logFileStreamString);
			logFileStreamString = "";
		}
	}
}

void guiLog::flushLogAtGame(int gameID) {

    if(myConfig->readConfigInt("LogOnOff")) {
       //	write for log after every game
        if(gameID > lastGameID) {
            writeLogFileStream(logFileStreamString);
            logFileStreamString = "";
            lastGameID = gameID;
        }
    }
}