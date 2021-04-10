This page provides information about contributing to Underworld’s codebase.

For contributions to Underworld models please go https://github.com/underworld-community

---- 

We welcome contributions to Underworld’s codebase in the form of:

* Code changes.
* Bug reports.
* Suggestions / Requests.
* Documentation modifications.

For Bug reports and Suggestions / Requests please submit an Issue on the Underworld GitHub Issue Tracker. Please tag the Issue with a given Label to help us assess the issue.

Click here to submit an Issue https://github.com/underworldcode/underworld2/issues


For Code / Documentation changes please submit a GitHub Pull Request (PR). This allows us to review and discuss the contributions before merging it into our ``development`` branch.
As part of the PR please provide a documentation change summary (for our ``Changes.md``) and a test where applicable.

For creating Pull Request (PR) we recommend following the workflow outlined https://guides.github.com/activities/forking/.
More specifically:

1. Fork Underworld via GitHub and clone it to your machine.

2. Add the original Underworld repository as `upstream` and branch your contribution off its development branch.

  .. code-block::
  
    git remote add upstream https://github.com/underworldcode/underworld2
    git checkout upstream/development ; git checkout -b newFeature
     
3. Make your changes! Remember to write comments, a test if applicable and follow the code style of the project (see    ``./docs/development/guidelines.md`` for details).
   We ask that a short description be made in the commit message that is appropriate for our ``Changes.md`` summary. If the change is sizeable or adds news functionality we ask for an associated blog post which describes the contribution in its full glory. Your time to shine! Details on how to write a blog post are coming soon.

4. Push your changes to your GitHub fork and then submit a PR to the ``development`` branch of Underworld via Github.
