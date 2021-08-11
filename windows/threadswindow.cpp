#include "threadswindow.hpp"
#include "../abaddon.hpp"

ThreadsWindow::ThreadsWindow(const ChannelData &channel)
    : m_channel_id(channel.ID)
    , m_box(Gtk::ORIENTATION_VERTICAL)
    , m_active(channel)
    , m_archived(channel) {
    set_name("threads-window");
    set_default_size(450, 375);
    set_title("#" + *channel.Name + " - Threads");
    set_position(Gtk::WIN_POS_CENTER);
    get_style_context()->add_class("app-window");
    get_style_context()->add_class("app-popup");
    get_style_context()->add_class("threads-window");

    const auto cb = [this](Snowflake id) {
        Abaddon::Get().ActionChannelOpened(id);
        hide();
    };
    m_active.signal_thread_open().connect(cb);
    m_archived.signal_thread_open().connect(cb);

    m_switcher.set_halign(Gtk::ALIGN_CENTER);
    m_switcher.set_stack(m_stack);

    m_stack.add(m_active, "active", "Active Threads");
    m_stack.add(m_archived, "archived", "Archived Threads");

    m_active.show();
    m_archived.show();
    m_switcher.show();
    m_stack.show();
    m_box.show();

    m_box.add(m_switcher);
    m_box.add(m_stack);
    add(m_box);
}

ThreadListRow::ThreadListRow(const ChannelData &channel)
    : ID(channel.ID)
    , m_label(*channel.Name, Gtk::ALIGN_START) {
    m_label.show();
    add(m_label);
}

ActiveThreadsList::ActiveThreadsList(const ChannelData &channel) {
    set_vexpand(true);

    m_list.set_selection_mode(Gtk::SELECTION_SINGLE);
    m_list.set_hexpand(true);
    m_list.show();

    add(m_list);

    m_list.signal_button_press_event().connect([this](GdkEventButton *ev) -> bool {
        if (ev->button == GDK_BUTTON_PRIMARY && ev->type == GDK_2BUTTON_PRESS) {
            if (auto row = dynamic_cast<ThreadListRow *>(m_list.get_selected_row()))
                m_signal_thread_open.emit(row->ID);
        }
        return false;
    });

    const auto threads = Abaddon::Get().GetDiscordClient().GetActiveThreads(channel.ID);
    for (const auto &thread : threads) {
        auto row = Gtk::manage(new ThreadListRow(thread));
        row->show();
        m_list.add(*row);
    }
}

ActiveThreadsList::type_signal_thread_open ActiveThreadsList::signal_thread_open() {
    return m_signal_thread_open;
}

ArchivedThreadsList::ArchivedThreadsList(const ChannelData &channel) {
    set_vexpand(true);

    m_list.set_selection_mode(Gtk::SELECTION_SINGLE);
    m_list.set_hexpand(true);
    m_list.show();

    add(m_list);

    m_list.signal_button_press_event().connect([this](GdkEventButton *ev) -> bool {
        if (ev->button == GDK_BUTTON_PRIMARY && ev->type == GDK_2BUTTON_PRESS) {
            if (auto row = dynamic_cast<ThreadListRow *>(m_list.get_selected_row()))
                m_signal_thread_open.emit(row->ID);
        }
        return false;
    });

    Abaddon::Get().GetDiscordClient().GetArchivedPublicThreads(channel.ID, sigc::mem_fun(*this, &ArchivedThreadsList::OnPublicFetched));
}

void ArchivedThreadsList::OnPublicFetched(DiscordError code, const ArchivedThreadsResponseData &data) {
    for (const auto &thread : data.Threads) {
        auto row = Gtk::manage(new ThreadListRow(thread));
        row->show();
        m_list.add(*row);
    }
}

ArchivedThreadsList::type_signal_thread_open ArchivedThreadsList::signal_thread_open() {
    return m_signal_thread_open;
}
