--
-- (C) 2018 - ntop.org
--

local email = {}

local function getEmailMessage(now_ts, from, to, subject, body)
  local now = os.date("%a, %d %b %Y %X", now_ts) -- E.g. "Tue, 3 Apr 2018 14:58:00"
  local msg_id = "<" .. now_ts .. "." .. os.clock() .. "@ntopng>"

  local lines = {
    "From: " .. from,
    "To: " .. to,
    "Subject: " .. subject,
    "Date: " ..  now,
    "Message-ID: " .. msg_id,
  }

  return table.concat(lines, "\r\n") .. "\r\n\r\n" .. body .. "\r\n"
end

function email.sendEmail(subject, message_body)
  local smtp_server = ntop.getPref("ntopng.prefs.alerts.smtp_server")
  local from_addr = ntop.getPref("ntopng.prefs.alerts.email_sender")
  local to_addr = ntop.getPref("ntopng.prefs.alerts.email_recipient")

  if isEmptyString(from_addr) or isEmptyString(to_addr) or isEmptyString(smtp_server) then
    return false
  end

  if not string.find(smtp_server, "://") then
    smtp_server = "smtp://" .. smtp_server
  end

  local parts = string.split(to_addr, "@")

  if #parts == 2 then
    local sender_domain = parts[2]
    smtp_server = smtp_server .. "/" .. sender_domain
  end

  local message = getEmailMessage(os.time(), from_addr, to_addr, subject, message_body)

  return ntop.sendMail(from_addr, to_addr, message, smtp_server)
end

function email.sendNotification(notif)
  local msg_prefix = alertNotificationActionToLabel(notif.action)
  local product = ntop.getInfo(false).product
  local subject = product .. " [" .. string.upper(notif.severity) .. "]: " .. alertTypeLabel(alertType(notif.type), true)
  local message_body = noHtml(msg_prefix .. notif.message)

  return email.sendEmail(subject, message_body)
end

return email