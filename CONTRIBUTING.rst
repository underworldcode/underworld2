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

For Code / Documentation changes please submit a GitHub Pull Request (PR). This allows us to review and discuss the contributions before merging it into our ``development`` branch. For creating Pull Request (PR) we recommend following the workflow outlined https://guides.github.com/activities/forking/.
More specifically:

1. Fork Underworld via GitHub and clone it to your machine.

  .. code-block::
  
    git clone https://github.com/YOUR_GITHUB_ACCOUNT/underworld2

2. Add the master Underworld repository as an additional remote source (named `uwmaster`) for your local repo and pull down its latest changesets. Checkout to the master/development repo state, and then create a new local branch which will contain your forthcoming changes.

  .. code-block::
  
    git remote add uwmaster https://github.com/underworldcode/underworld2
    git pull uwmaster
    git checkout uwmaster/development
    git checkout -b newFeature
     
3. Make your changes! Remember to write comments, a test if applicable and follow the code style of the project (see ``./docs/development/guidelines.md`` for details).

4. Push your changes to your GitHub fork and then submit a PR to the ``development`` branch of Underworld via Github.
