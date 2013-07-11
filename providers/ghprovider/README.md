
Github Provider
===============

This plugin allows KDevelop to fetch repositories hosted on
[Github](https://github.com). It's able to fetch both private and public
repositories from users and from organizations.


Basics
------

When the user wants to fetch a project from Github, it just selects this
plugin in the "Source" combobox in the "Fetch Project" widget. Afterwards
the user should select if he's looking for another user's repository, or for
a repository from an organization. He can then search for the repository
and fetch it by clicking the "Get" button. By using this plugin this way,
this plugin will only list public repositories.


Authorized users
----------------

This plugin allows the user to authorize KDevelop to use his account. This
way, this plugin will be able to list also private repositories. After
configuring the Github account, the user will be able to list his own
repositories (including private ones) and the repositories from organizations
that this user belongs to, by selecting it on the combobox.
