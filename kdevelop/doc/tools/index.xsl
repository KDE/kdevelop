<?xml version="1.0"?>
<xsl:stylesheet version="1.0"
                xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output omit-xml-declaration="yes"/>

<xsl:template match="/"><xsl:apply-templates/></xsl:template>

<xsl:template match="text()|@*"></xsl:template>

<xsl:template match="chapter">
  <xsl:variable name="url" select="@id"/>
  <xsl:apply-templates/>   
</xsl:template>  

<xsl:template match="chapter/section[position()!=1]">
  <xsl:variable name="url" select="@id"/>
  <xsl:apply-templates/>   
</xsl:template>  

<xsl:template match="indexterm">
  <xsl:choose>
    <xsl:when test="./secondary and ./tertiary">
    <entry name="{primary}, {secondary}, {tertiary}" url="{$url}.html"/>
    </xsl:when>
    <xsl:when test="./secondary">
    <entry name="{primary}, {secondary}" url="{$url}.html"/>
    </xsl:when>
    <xsl:otherwise>
    <entry name="{primary}" url="{$url}.html"/>
    </xsl:otherwise>
  </xsl:choose>
</xsl:template>

</xsl:stylesheet>
