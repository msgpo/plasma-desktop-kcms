/*
 *   Copyright 2017 Marco Martin <mart@kde.org>
 *   Copyright 2020 Dominic Hayes <ferenosdev@outlook.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "kcm.h"

#include <iostream>

#include <qcommandlineparser.h>
#include <QApplication>
#include <QDebug>

// Frameworks
#include <KAboutData>
#include <klocalizedstring.h>

#include <KPackage/Package>
#include <KPackage/PackageLoader>

#include "lookandfeelsettings.h"

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    const char version[] = "1.0";

    // About data
    KAboutData aboutData("lookandfeeltool", i18n("Global Theme Tool"), version, i18n("Command line tool to apply global theme packages for changing the look and feel."), KAboutLicense::GPL, i18n("Copyright 2017, Marco Martin, Copyright 2020, Dominic Hayes"));
    aboutData.addAuthor(i18n("Marco Martin"), i18n("Maintainer"), QStringLiteral("mart@kde.org"));
    aboutData.setDesktopFileName("org.kde.lookandfeeltool");
    KAboutData::setApplicationData(aboutData);

    const static auto _l = QStringLiteral("list");
    const static auto _a = QStringLiteral("apply");
    const static auto _r = QStringLiteral("resetLayout");

    QCommandLineOption _list = QCommandLineOption(QStringList() << QStringLiteral("l") << _l,
                               i18n("List available global theme packages"));
    QCommandLineOption _apply = QCommandLineOption(QStringList() << QStringLiteral("a") << _a,
                                i18n("Apply a global theme package"), i18n("packagename"));
    QCommandLineOption _resetLayout = QCommandLineOption(QStringList() << 
                               _r, i18n("Reset the Plasma Desktop layout"));

    QCommandLineParser parser;
    parser.addOption(_list);
    parser.addOption(_apply);
    parser.addOption(_resetLayout);
    aboutData.setupCommandLine(&parser);

    parser.process(app);
    aboutData.processCommandLine(&parser);

    if (!parser.isSet(_list) && !parser.isSet(_apply)) {
        parser.showHelp();
    }
    
    if (parser.isSet(_list)) {
        QList<KPluginMetaData> pkgs = KPackage::PackageLoader::self()->listPackages("Plasma/LookAndFeel");

        for (const KPluginMetaData &data : pkgs) {
            std::cout << data.pluginId().toStdString() << std::endl;
        }

    } else if (parser.isSet(_apply)) {
        KPackage::Package p = KPackage::PackageLoader::self()->loadPackage("Plasma/LookAndFeel");
        p.setPath(parser.value(_apply));

        //can't use package.isValid as lnf packages always fallback, even when not existing
        if (p.metadata().pluginId() != parser.value(_apply)) {
            std::cout << "Unable to find the theme named " << parser.value(_apply).toStdString() << std::endl;
            return 1;
        }

        KCMLookandFeel *kcm = new KCMLookandFeel(nullptr, QVariantList());
        kcm->load();
        kcm->lookAndFeelSettings()->setGlobalThemePackage(parser.value(_apply));
        kcm->save();
        if (parser.isSet(_resetLayout)) {
            std::string laftheme = parser.value(_apply).toStdString();
            std::system(("/usr/bin/desktoplayouttool -a " + laftheme).c_str());
        }
        // TODO: figure out why setGlobalThemePackage isn't setting GlobalThemePackage
        KConfig config(QStringLiteral("kdeglobals"));
        KConfigGroup cg(&config, "KDE");
        cg.writeEntry("GlobalThemePackage", parser.value(_apply));
        cg.sync();
        delete kcm;
    }

    return 0;
}

