;;; collect.el -- The GNU Emacs script to run on the Aris grading server.

;; Copyright (C) 2013 Ian Dunn

;; Author: Ian Dunn

;; GNU Aris is free software; you can redistribute it and/or modify it
;; under the terms of the GNU General Public License as published by
;; the Free Software Foundation; either version 3, or (at your option)
;; any later version.

;; GNU Aris is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
;; or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
;; License for more details.

;; You should have received a copy of the GNU General Public License
;; along with Aris; see the file COPYING.  If not, write to the Free
;; Software Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
;; 02110-1301, USA.

;; These variables may need to be set, depending on the server and user.

(setq send-mail-function (quote smtpmail-send-it))
(setq smtpmail-smtp-server "FIX ME")
(setq smtpmail-smtp-service 25)
(setq smtpmail-smtp-user "USER NAME HERE")
(setq smtpmail-stream-type (quote plain))
(setq user-mail-address "USER@THINGS.org")

(defun send-aris-response (user-email message &optional instructor)
  (mail)

  (mail-to)
  (insert user-email)

  (mail-subject)
  (insert (concat "Grade Report for " user-email))

  (goto-char 7)
  (kill-line)
  (insert "Aris Grader <aris-grade@rpi.edu>")

  (if instructor
      (progn
	(mail-cc)
	(insert instructor)))

  (goto-char (point-max))
  (insert "\n")
  (insert message)
  (mail-send))
