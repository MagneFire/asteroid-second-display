install(FILES ../systemd/asteroid-seconddisplayd.service
    DESTINATION /lib/systemd/system)

install(FILES ../systemd/asteroid-seconddisplay-sync.service
    DESTINATION /lib/systemd/system)

install(FILES ../systemd/asteroid-seconddisplay-sync.timer
    DESTINATION /lib/systemd/system)