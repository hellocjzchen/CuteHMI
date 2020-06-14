#include "../cutehmi.metadata.hpp"
#include "../cutehmi.dirs.hpp"
#include "cutehmi/view/logging.hpp"

#include <cutehmi/ErrorInfo.hpp>
#include <cutehmi/Message.hpp>
#include <cutehmi/Messenger.hpp>
#include <cutehmi/Singleton.hpp>

#include <cutehmi/gui/CuteApplication.hpp>

//<cutehmi.view.2-4.workaround target="Qt" cause="bug">
#include <QApplication>
// Instead of:
//  #include <QGuiApplication>
//</cutehmi.view.2-4.workaround>

#include <QQmlApplicationEngine>
#include <QIcon>
#include <QDir>
#include <QtDebug>
#include <QCommandLineParser>
#include <QQmlContext>
#include <QUrl>
#include <QCursor>
#include <QTranslator>
#include <QLibraryInfo>
#include <QFile>

using namespace cutehmi::view;

/**
 * Main function.
 * @param argc number of arguments passed to the program.
 * @param argv list of arguments passed to the program.
 * @return return code.
 */
int main(int argc, char * argv[])
{
#ifdef CUTEHMI_VIEW_DEFAULT_INIT
	static constexpr const char * DEFAULT_INIT = CUTEHMI_VIEW_DEFAULT_INIT;
#else
	static constexpr const char * DEFAULT_INIT = "qrc:/qml/ExtensionLoader.qml";
#endif
#ifdef CUTEHMI_VIEW_DEFAULT_EXTENSION
	static constexpr const char * DEFAULT_EXTENSION = CUTEHMI_VIEW_DEFAULT_EXTENSION;
#else
	static constexpr const char * DEFAULT_EXTENSION = "";
#endif
#ifdef CUTEHMI_VIEW_DEFAULT_COMPONENT
	static constexpr const char * DEFAULT_COMPONENT = CUTEHMI_VIEW_DEFAULT_COMPONENT;
#else
	static constexpr const char * DEFAULT_COMPONENT = "View";
#endif
#ifdef CUTEHMI_VIEW_DEFAULT_MINOR
	static constexpr const char * DEFAULT_MINOR = CUTEHMI_VIEW_DEFAULT_MINOR;
#else
	static constexpr const char * DEFAULT_MINOR = "0";
#endif

	QCoreApplication::setOrganizationName(CUTEHMI_VIEW_VENDOR);
	QCoreApplication::setOrganizationDomain(CUTEHMI_VIEW_DOMAIN);
	QCoreApplication::setApplicationName(CUTEHMI_VIEW_FRIENDLY_NAME);
	QCoreApplication::setApplicationVersion(QString("%1.%2.%3").arg(CUTEHMI_VIEW_MAJOR).arg(CUTEHMI_VIEW_MINOR).arg(CUTEHMI_VIEW_MICRO));

	try {

#ifdef CUTEHMI_VIEW_VIRTUAL_KEYBOARD
		if (qgetenv("QT_IM_MODULE").isEmpty())
			qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
		CUTEHMI_DEBUG("Input method: " << qgetenv("QT_IM_MODULE"));
#else
		CUTEHMI_DEBUG("Support for virtual keyboard is disabled.");
#endif

		if (qgetenv("QT_IM_MODULE") == "qtvirtualkeyboard") {
			if (qgetenv("QT_VIRTUALKEYBOARD_LAYOUT_PATH").isEmpty())
				qputenv("QT_VIRTUALKEYBOARD_LAYOUT_PATH", QByteArray(QDir("../layouts").absolutePath().toLocal8Bit()));
			CUTEHMI_DEBUG("Qt Virtual Keyboard layouts path: " << qgetenv("QT_VIRTUALKEYBOARD_LAYOUT_PATH"));
		}

		//<Qt-Qt_5_7_0_Reference_Documentation-Threads_and_QObjects-QObject_Reentrancy-creating_QObjects_before_QApplication.assumption>
		// "In general, creating QObjects before the QApplication is not supported and can lead to weird crashes on exit, depending on the
		//	platform. This means static instances of QObject are also not supported. A properly structured single or multi-threaded application
		//	should make the QApplication be the first created, and last destroyed QObject."

		//<cutehmi.view.2-4.workaround target="Qt" cause="bug">
		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		cutehmi::gui::CuteApplication app(argc, argv);
		// Instead of:
		//	QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		//	QGuiApplication app(argc, argv);
		//</cutehmi.view.2-4.workaround>
		app.setWindowIcon(QIcon(":/img/icon.png"));


		QString language = QLocale::system().name();
#ifdef CUTEHMI_VIEW_DEFAULT_LANGUAGE
		language = CUTEHMI_VIEW_DEFAULT_LANGUAGE;
#endif
		if (!qgetenv("CUTEHMI_LANGUAGE").isEmpty()) {
			language = qgetenv("CUTEHMI_LANGUAGE");
			CUTEHMI_DEBUG("Language set by 'CUTEHMI_LANGUAGE' environmental variable: " << qgetenv("CUTEHMI_LANGUAGE"));
		}
		QTranslator qtTranslator;
		if (!qtTranslator.load("qt_" + language, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
			CUTEHMI_WARNING("Could not load translations file '" << "qt_" + language << "'.");
		app.installTranslator(& qtTranslator);

		QTranslator viewTranslator;
		QString translationsDir = QDir("/" CUTEHMI_DIRS_TOOLS_INSTALL_SUBDIR).relativeFilePath("/" CUTEHMI_DIRS_TRANSLATIONS_INSTALL_SUBDIR);
		QString viewTranslationFilePrefix = QString(CUTEHMI_VIEW_NAME).replace('.', '-') + "_";
		QString viewTranslationFile = viewTranslationFilePrefix + language;
		if (!viewTranslator.load(viewTranslationFile, translationsDir))
			CUTEHMI_WARNING("Could not load translations file '" << viewTranslationFile << "'.");
		app.installTranslator(& viewTranslator);


		QCommandLineParser cmd;
		cmd.setApplicationDescription(CUTEHMI_VIEW_TRANSLATED_FRIENDLY_NAME + "\n" + CUTEHMI_VIEW_TRANSLATED_DESCRIPTION);
		cmd.addHelpOption();
		cmd.addVersionOption();

		QCommandLineOption fullScreenOption({"f", "fullscreen"}, QCoreApplication::translate("main", "Run application in full screen mode."));
		cmd.addOption(fullScreenOption);

		QCommandLineOption initOption("init", QCoreApplication::translate("main", "Override loader by specifying initial QML <file> to load."), QCoreApplication::translate("main", "file"));
		initOption.setDefaultValue(DEFAULT_INIT);
#ifdef CUTEHMI_VIEW_FORCE_DEFAULT_OPTIONS
		initOption.setFlags(QCommandLineOption::HiddenFromHelp);
#endif
		cmd.addOption(initOption);

		QCommandLineOption minorOption({"m", "minor"}, QCoreApplication::translate("main", "Use <version> for extension minor version to import."), QCoreApplication::translate("main", "version"));
		minorOption.setDefaultValue(DEFAULT_MINOR);
#ifdef CUTEHMI_VIEW_FORCE_DEFAULT_OPTIONS
		minorOption.setFlags(QCommandLineOption::HiddenFromHelp);
#endif
		cmd.addOption(minorOption);

		QCommandLineOption hideCursorOption({"t", "touch"}, QCoreApplication::translate("main", "Touch screen (hides mouse cursor)."));
		cmd.addOption(hideCursorOption);

		QCommandLineOption styleOption("qstyle", QCoreApplication::translate("main", "Set Qt Quick <style>."), QCoreApplication::translate("main", "style"));
		cmd.addOption(styleOption);

		QCommandLineOption langOption("lang", QCoreApplication::translate("main", "Choose application <language>."), QCoreApplication::translate("main", "language"));
		langOption.setDefaultValue(language);
		cmd.addOption(langOption);

		QCommandLineOption basedirOption("basedir", QCoreApplication::translate("main", "Set base directory to <dir>."), QCoreApplication::translate("main", "dir"));
		cmd.addOption(basedirOption);

#ifndef CUTEHMI_VIEW_FORCE_DEFAULT_OPTIONS
		cmd.addPositionalArgument("extension", QCoreApplication::translate("main", "Extension to import."), "[extension]");

		cmd.addPositionalArgument("component", QCoreApplication::translate("main", "Component to create. Defaults to 'View'."), "[component]");
#endif

		cmd.process(app);


		CUTEHMI_DEBUG("Default locale: " << QLocale());
		CUTEHMI_DEBUG("Language: " << cmd.value(langOption));

		if (!qtTranslator.load("qt_" + cmd.value(langOption), QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
			CUTEHMI_WARNING("Could not load translations file '" << "qt_" + cmd.value(langOption) << "'.");

		viewTranslationFile = viewTranslationFilePrefix + cmd.value(langOption);
		if (!viewTranslator.load(viewTranslationFile, translationsDir))
			CUTEHMI_WARNING("Could not load translations file '" << viewTranslationFile << "'.");


		if (cmd.isSet(styleOption)) {
			qputenv("QT_QUICK_CONTROLS_STYLE", cmd.value(styleOption).toLocal8Bit());
			CUTEHMI_DEBUG("Qt Quick style: " << cmd.value(styleOption));
		}

		if (cmd.isSet(hideCursorOption))
			QGuiApplication::setOverrideCursor(QCursor(Qt::BlankCursor));
		//<cutehmi.view.2-5.workaround target="Qt" cause="bug">
		// When run on raw Xorg server application does not show up cursor unless some controls are hovered.
		//<cutehmi.view.2-6.workaround target="Qt" cause="bug">
		// On Windows cursor does not refresh properly when using QGuiApplication::setOverrideCursor().
#ifndef Q_OS_WIN
		else
			QGuiApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
#endif
		//</cutehmi.view.2-6.workaround>
		//</cutehmi.view.2-5.workaround>

		QDir baseDir(QCoreApplication::applicationDirPath() + "/..");
		if (cmd.isSet(basedirOption))
			baseDir.setPath(cmd.value(basedirOption));
		QString baseDirPath = baseDir.absolutePath() + "/";
		CUTEHMI_DEBUG("Base directory: " << baseDirPath);

		CUTEHMI_DEBUG("Library paths: " << QCoreApplication::libraryPaths());

		std::unique_ptr<QQmlApplicationEngine> engine(new QQmlApplicationEngine);
		engine->addImportPath(baseDirPath + CUTEHMI_DIRS_EXTENSIONS_INSTALL_SUBDIR);

		CUTEHMI_DEBUG("QML import paths: " << engine->importPathList());

		QStringList positionalArguments = cmd.positionalArguments();
#ifndef CUTEHMI_VIEW_FORCE_DEFAULT_OPTIONS
		QString extension;
		if (positionalArguments.length() > 0)
			extension = positionalArguments.at(0);
		else
			extension = DEFAULT_EXTENSION;

		QString extensionMinor = cmd.value(minorOption);
		QString init = cmd.value(initOption);
		QString component;
		if (positionalArguments.length() > 1)
			component = positionalArguments.at(1);
		else
			component = DEFAULT_COMPONENT;
#else
		QString extensionMinor = minorOption.defaultValues().first();
		QString init = initOption.defaultValues().first();
		QString extension = DEFAULT_EXTENSION;
		QString component = DEFAULT_COMPONENT;

		if (positionalArguments.length() > 1)
			CUTEHMI_DIE(QCoreApplication::translate("main", "You can not use 'component' option, because 'forceDefaultOptions' option has been set during compilation time.").toLocal8Bit().constData());
		if (positionalArguments.length() > 0)
			CUTEHMI_DIE(QCoreApplication::translate("main", "You can not use 'extension' option, because 'forceDefaultOptions' option has been set during compilation time.").toLocal8Bit().constData());
		if (cmd.value(minorOption) != extensionMinor)
			CUTEHMI_DIE(QCoreApplication::translate("main", "You can not use '%1' option, because 'forceDefaultOptions' option has been set during compilation time.").arg(minorOption.names().join(", ")).toLocal8Bit().constData());
		if (cmd.value(initOption) != init)
			CUTEHMI_DIE(QCoreApplication::translate("main", "You can not use '%1' option, because 'forceDefaultOptions' option has been set during compilation time.").arg(initOption.names().join(", ")).toLocal8Bit().constData());
#endif
		if (!extensionMinor.isEmpty()) {
			bool ok;
			extensionMinor.toUInt(& ok);
			if (!ok)
				CUTEHMI_DIE(QCoreApplication::translate("main", "Command line argument error: value of '%1' option must be a number.").arg(minorOption.names().last()).toLocal8Bit().constData());
		}
		QString extensionBaseName = extension.left(extension.lastIndexOf('.'));
		QString extensionMajor = extension.right(extension.length() - extension.lastIndexOf('.') - 1);
		engine->rootContext()->setContextProperty("cutehmi_view_extensionBaseName", extensionBaseName);
		engine->rootContext()->setContextProperty("cutehmi_view_extensionMajor", extensionMajor);
		engine->rootContext()->setContextProperty("cutehmi_view_extensionMinor", extensionMinor);
		engine->rootContext()->setContextProperty("cutehmi_view_extensionComponent", component);
		engine->rootContext()->setContextProperty("cutehmi_view_initURL", "qrc:/qml/DefaultScreen.qml");

		engine->load(QUrl(QStringLiteral("qrc:/qml/MainWindow.qml")));

		if (!init.isNull()) {
			CUTEHMI_DEBUG("Init: '" << init << "'");
			CUTEHMI_DEBUG("Extension: '" << extension << "'");
			CUTEHMI_DEBUG("Minor: '" << extensionMinor << "'");
			CUTEHMI_DEBUG("Component: '" << component << "'");
			QUrl initUrl(init);
			if (initUrl.isValid()) {
				// Assure that URL is not mixing relative path with explicitly specified scheme, which is forbidden. QUrl::isValid() doesn't check this out.
				if (!initUrl.scheme().isEmpty() && QDir::isRelativePath(initUrl.path()))
					cutehmi::Message::Critical(QCoreApplication::translate("main", "URL '%1' contains relative path along with URL scheme, which is forbidden.").arg(initUrl.url()));
				else {
					// If source URL is relative (does not contain scheme), then make absolute URL: file:///baseDirPath/sourceUrl.
					if (initUrl.isRelative())
						initUrl = QUrl::fromLocalFile(baseDirPath).resolved(initUrl);
					// Check if file exists and eventually set context property.
					if (initUrl.isLocalFile() && !QFile::exists(initUrl.toLocalFile()))
						cutehmi::Message::Critical(QCoreApplication::translate("main", "QML file '%1' does not exist.").arg(initUrl.url()));
					else
						engine->rootContext()->setContextProperty("cutehmi_view_initURL", initUrl.url());
				}
			} else
				cutehmi::Message::Critical(QCoreApplication::translate("main", "Invalid format of QML file URL '%1'.").arg(init));
		}

		//<Qt-Qt_5_9_1_Reference_Documentation-Qt_Core-C++_Classes-QCoreApplication-exec.assumption>
		// "We recommend that you connect clean-up code to the aboutToQuit() signal, instead of putting it in your application's main() function because on some
		//  platforms the exec() call may not return. For example, on Windows when the user logs off, the system terminates the process after Qt closes all top-level
		//  windows. Hence, there is no guarantee that the application will have time to exit its event loop and execute code at the end of the main() function after
		//  the exec() call."
		QObject::connect(& app, & cutehmi::gui::CuteApplication::aboutToQuit, [&]() {
			// It's quite important to destroy "engine" before cutehmi::CuteHMI::Instance() members, because they
			// may still be used by some QML components (for example in "Component.onDestroyed" handlers).
			engine.reset();

			cutehmi::destroySingletonInstances();

			if (cmd.isSet(hideCursorOption))
				QGuiApplication::restoreOverrideCursor();
			//<cutehmi.view.2-5.workaround>
			//<cutehmi.view.2-6.workaround target="Qt" cause="bug">
			// On Windows cursor does not refresh properly when using QGuiApplication::setOverrideCursor().
#ifndef Q_OS_WIN
			else
				QGuiApplication::restoreOverrideCursor();
#endif
			//</cutehmi.view.2-6.workaround>
			//</cutehmi.view.2-5.workaround>
		});

		return app.exec();

		//</Qt-Qt_5_9_1_Reference_Documentation-Qt_Core-C++_Classes-QCoreApplication-exec.assumption>

	} catch (const cutehmi::Messenger::NoAdvertiserException & e) {
		CUTEHMI_CRITICAL("Dialog message: " << e.message()->text());
		if (!e.message()->informativeText().isEmpty())
			CUTEHMI_CRITICAL("Informative text: " << e.message()->informativeText());
		if (!e.message()->detailedText().isEmpty())
			CUTEHMI_CRITICAL("Detailed text: " << e.message()->detailedText());
		CUTEHMI_CRITICAL("Available buttons: " << e.message()->buttons());
	} catch (const QException & e) {
		CUTEHMI_CRITICAL(e.what());
	} catch (const std::exception & e) {
		CUTEHMI_CRITICAL(e.what());
	} catch (...) {
		CUTEHMI_CRITICAL("Caught unrecognized exception.");
		throw;
	}

	return  EXIT_FAILURE;

	//</Qt-Qt_5_7_0_Reference_Documentation-Threads_and_QObjects-QObject_Reentrancy-creating_QObjects_before_QApplication.assumption>
}

//(c)C: Copyright © 2020, Michał Policht <michal@policht.pl>. All rights reserved.
//(c)C: This file is a part of CuteHMI.
//(c)C: CuteHMI is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.
//(c)C: CuteHMI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.
//(c)C: You should have received a copy of the GNU Lesser General Public License along with CuteHMI.  If not, see <https://www.gnu.org/licenses/>.