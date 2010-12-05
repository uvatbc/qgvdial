[Profile]
DisplayName = Google Voice
#IconName = vicar_phone
Manager = qgvtp
Protocol = qgv
Priority = -1
VCardDefault = 1
VCardField = X-QGV
SecondaryVCardFields = TEL
Capabilities = voice-p2p
ConfigurationUI = osso-accounts
LocalizationDomain = rtcom-call-ui

[Action call]
Name = Call via QGV
#IconName = vicar_phone
VCardFields = X-QGV,TEL
prop-org.freedesktop.Telepathy.Channel.ChannelType-s = org.freedesktop.Telepathy.Channel.Type.StreamedMedia

[Action chat]
Name = Text via QGV
#IconName = vicar_phone
VCardFields = X-QGV,TEL
prop-org.freedesktop.Telepathy.Channel.ChannelType-s = org.freedesktop.Telepathy.Channel.Type.Text

