# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'nRF70 bare metal library'
copyright = "2024, Nordic Semiconductor"
author = "Nordic Semiconductor ASA"
release = '1.0.0'

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
    'breathe',
]

breathe_projects = {
    'nrf70_bm_lib': '../xml',
    'nrfxlib': '../xml'
}
breathe_default_project = 'nrf70_bm_lib'
breathe_domain_by_extension = {"h": "c", "c": "c"}
breathe_separate_member_pages = True

templates_path = ['_templates']
exclude_patterns = []

# Ensure the master doc is set correctly
master_doc = 'index'
source_suffix = '.rst'

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

html_theme = 'sphinx_rtd_theme'
