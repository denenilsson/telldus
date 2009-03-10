#include "mainwindow.h"

#include <QApplication>
#include <QMenuBar>
#include <QStackedLayout>
#include <QStatusBar>
#include <QToolBar>
#include <QMessageBox>
#include <QSettings>
#include <QVBoxLayout>

#include <QDebug>

#include "tellduscenterapplication.h"
#include "tellduscenterplugin.h"
#include "message.h"
#include "../TelldusGui/telldusgui.h"

class MainWindowPrivate {
public:
	QToolBar *pagesBar;
	Message *message;
	QStackedLayout *stackedLayout;
};

MainWindow::MainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	d = new MainWindowPrivate;
	d->message = new Message(this);

	setAttribute(Qt::WA_DeleteOnClose, true);

	// Restore size and position
	QSettings settings;
	resize(settings.value("Size", size()).toSize());
	move(settings.value("Pos", pos()).toPoint());

	statusBar()->setSizeGripEnabled(true);
	setupMenu();

	QWidget *centralWidget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout;
	centralWidget->setLayout(layout);

	layout->addWidget(d->message);

	d->stackedLayout = new QStackedLayout;
	layout->addLayout(d->stackedLayout);

	TelldusCenterApplication *app = TelldusCenterApplication::instance();
	QWidget *deviceWidget = tdDeviceWidget(this);
	connect(deviceWidget, SIGNAL(showMessage(const QString &, const QString &, const QString &)), app, SLOT(showMessage(const QString &, const QString &, const QString &)));
	connect(deviceWidget, SIGNAL(eventTriggered(const QString &, const QString &)), app, SLOT(eventTriggered(const QString &, const QString &)));
	d->stackedLayout->addWidget(deviceWidget);

	setCentralWidget(centralWidget);

	setupToolBar();

	setWindowTitle( tr("Telldus Center") );
}

MainWindow::~MainWindow()
{
	delete d;
}

void MainWindow::showMessage( const QString &title, const QString &message, const QString &detailedMessage ) {
	d->message->showMessage( title, message, detailedMessage );
}

void MainWindow::closeEvent( QCloseEvent */*event*/ ) {
	QSettings settings;
	settings.setValue("Size", size());
	settings.setValue("Pos", pos());

}

void MainWindow::setupMenu()
{
	menuBar()->clear();

	// File
//	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	// Help
	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addSeparator();
	helpMenu->addAction(tr("About &Qt"), qApp, SLOT(aboutQt()));
	helpMenu->addAction(tr("About &Telldus Center"), this, SLOT(slotAboutApplication()));
}

void MainWindow::setupToolBar()
{
	setUnifiedTitleAndToolBarOnMac(true);
	d->pagesBar = addToolBar(tr("Pages"));
	d->pagesBar->setIconSize(QSize(32, 32));

	QActionGroup *ag = new QActionGroup(this);

	QSet<QString> toolbarIcons;
	QAction *actionDevices = new QAction( QIcon(":/images/devices.png"), tr("Devices"), this );
	actionDevices->setCheckable( true );
	actionDevices->setChecked( true );
	actionDevices->setData(0);
	connect(actionDevices, SIGNAL(triggered()), this, SLOT(slotPagesClick()));
	ag->addAction(actionDevices);
	toolbarIcons.insert("devices");

	TelldusCenterApplication *app = TelldusCenterApplication::instance();
	PluginList plugins = app->plugins();
	foreach( TelldusCenterPlugin *plugin, plugins ) {

		QStringList widgets = plugin->widgets();
		foreach( QString widget, widgets ) {
			QString page = widget.section('.', 0, 0);
			if (!toolbarIcons.contains( page )) {
				QWidget *pageWidget = plugin->widget( page, this );
				if (!pageWidget) {
					continue;
				}
				QAction *action = new QAction( plugin->iconForPage( page ), page, this );
				action->setCheckable( true );
				action->setChecked( false );

				int index = d->stackedLayout->addWidget( pageWidget );
				action->setData( index );

				connect(action, SIGNAL(triggered()), this, SLOT(slotPagesClick()));
				ag->addAction( action );
				toolbarIcons.insert(page);
			}
		}

	}
	d->pagesBar->addActions( ag->actions() );
}

void MainWindow::slotAboutApplication() {
	QMessageBox::about(this, tr("About Telldus Center"),
					   tr("<h2>Telldus Center 0.1</h2>"
						  "<p>Copyright &copy; 2008 Telldus Technologies AB<p>"
						  "<p>Telldus Center is a configuration utility for Telldus TellStick&reg;</p>"));
}

void MainWindow::slotPagesClick() {
	QAction *action = qobject_cast<QAction *>(sender());
	if (action) {
		d->stackedLayout->setCurrentIndex(action->data().toInt());
	}
}