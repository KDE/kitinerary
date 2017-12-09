<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
    <xsl:output method="text" encoding="utf8"/>
    <xsl:template match="/qgis">
        <xsl:apply-templates select="//symbols/symbol">
            <xsl:sort select="@name" data-type="number"/>
        </xsl:apply-templates>
    </xsl:template>

    <xsl:template match="symbol">
        <xsl:variable name="name" select="@name" />
        <xsl:value-of select="concat(layer/prop[@k = 'color']/@v, ',', ../../categories/category[@symbol = $name]/@value, '&#10;')" />
    </xsl:template>
</xsl:stylesheet>
