/* This file is part of the KDE Project
   Copyright (c) 2014 Vishesh Handa <me@vhanda.in>
   Copyright (c) 2020 Alexander Lohnau <alexander.lohnau@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcm.h"

#include <KPluginFactory>
#include <KAboutData>
#include <KSharedConfig>
#include <QDebug>
#include <QStandardPaths>
#include <KLocalizedString>
#include <KRunner/RunnerManager>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KPluginSelector>

#include <QApplication>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialog>
#include <QPainter>
#include <QPushButton>

K_PLUGIN_FACTORY(SearchConfigModuleFactory, registerPlugin<SearchConfigModule>();)


SearchConfigModule::SearchConfigModule(QWidget* parent, const QVariantList& args)
    : KCModule(parent, args)
    , m_config("krunnerrc")
{
    KAboutData* about = new KAboutData(QStringLiteral("kcm_search"), i18nc("kcm name for About dialog", "Configure Search Bar"),
                                       QStringLiteral("0.1"), QString(), KAboutLicense::LGPL);
    about->addAuthor(i18n("Vishesh Handa"), QString(), QStringLiteral("vhanda@kde.org"));
    setAboutData(about);
    setButtons(Apply | Default);

    if(!args.at(0).toString().isEmpty()) {
        m_pluginID = args.at(0).toString();
    }

    QVBoxLayout* layout = new QVBoxLayout(this);

    QHBoxLayout *headerLayout = new QHBoxLayout(this);

    QLabel *label = new QLabel(i18n("Enable or disable KRunner plugins:"));

    QPushButton *clearHistoryButton = new QPushButton(i18n("Clear History"));
    clearHistoryButton->setIcon(QIcon::fromTheme(isRightToLeft() ? QStringLiteral("edit-clear-locationbar-ltr")
                                                                 : QStringLiteral("edit-clear-locationbar-rtl")));
    connect(clearHistoryButton, &QPushButton::clicked, this, [this] {
        KConfigGroup generalConfig(m_config.group("General"));
        generalConfig.deleteEntry("history", KConfig::Notify);
        generalConfig.sync();
    });

    headerLayout->addWidget(label);
    headerLayout->addStretch();
    headerLayout->addWidget(clearHistoryButton);

    m_pluginSelector = new KPluginSelector(this);

    connect(m_pluginSelector, &KPluginSelector::changed, this, &SearchConfigModule::markAsChanged);
    connect(m_pluginSelector, &KPluginSelector::configCommitted, this, &SearchConfigModule::markAsChanged);
    connect(m_pluginSelector, &KPluginSelector::defaulted, this, &KCModule::defaulted);

    layout->addLayout(headerLayout);
    layout->addWidget(m_pluginSelector);
}

void SearchConfigModule::load()
{
    // Set focus on the pluginselector to pass focus to search bar.
    m_pluginSelector->setFocus(Qt::OtherFocusReason);

    m_pluginSelector->addPlugins(Plasma::RunnerManager::listRunnerInfo(),
                    KPluginSelector::ReadConfigFile,
                    i18n("Available Plugins"), QString(),
                    KSharedConfig::openConfig(QStringLiteral( "krunnerrc" )));
    m_pluginSelector->load();

    if(!m_pluginID.isEmpty()){
        m_pluginSelector->showConfiguration(m_pluginID);
    }
}


void SearchConfigModule::save()
{
    m_pluginSelector->save();
}

void SearchConfigModule::defaults()
{
    m_pluginSelector->defaults();
}

#include "kcm.moc"
