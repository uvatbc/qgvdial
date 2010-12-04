[Profile]
DisplayName = QGV
#IconName = vicar_phone
Manager = qgvtp
Protocol = qgv
Priority = -1
VCardDefault = 1
VCardField = X-QGV
Capabilities = voice-p2p
ConfigurationUI = osso-accounts
LocalizationDomain = rtcom-call-ui

[Action call]
Name = Call via QGV
#IconName = vicar_phone
VCardFields = X-QGV
prop-org.freedesktop.Telepathy.Channel.ChannelType-s = org.freedesktop.Telepathy.Channel.Type.StreamedMedia

